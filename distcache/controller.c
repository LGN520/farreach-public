#include <arpa/inet.h>  // inetaddr conversion
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>  // struct sockaddr_in
#include <signal.h>      // for signal and raise
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // socket API
#include <sys/stat.h>
#include <sys/time.h>  // struct timeval
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <string>
#include <vector>

// CPU affinity
#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>

#include "../common/helper.h"
#include "../common/io_helper.h"

#include "common_impl.h"

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
size_t controller_expected_ready_threads = -1;

// cache population/eviction

// switchos.popworker.popclient_for_controller <-> controller.popserver
int controller_popserver_udpsock = -1;
// controller.popclient <-> server.popserver
int controller_popserver_popclient_udpsock = -1;
// TODO: replace with SIGTERM
bool volatile is_controller_popserver_finish = false;

// switchos.popworker <-> controller.evictserver
int controller_evictserver_udpsock = -1;
// controller.evictclients <-> servers.evictservers
// NOTE: evictclient.index = serveridx
// bool volatile is_controller_evictserver_evictclients_connected = false;
// int * controller_evictserver_evictclient_tcpsock_list = NULL;
// controller.evictclient <-> server.evictserver
int controller_evictserver_evictclient_udpsock = -1;

void prepare_controller();
void* run_controller_popserver(void* param);    // Receive NETCACHE_CACHE_POP from switchos
void* run_controller_evictserver(void* param);  // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void close_controller();

cpu_set_t nonserverworker_cpuset;  // [server_cores, total_cores-1] for all other threads

uint32_t controller_idx = 0;

int main(int argc, char** argv) {
    if (argc == 2) {
        // printf("Usage: ./controller controller_idx\n");
        controller_idx = atoi(argv[1]);
    } 
    
    parse_ini("config.ini", controller_idx);
    parse_control_ini("control_type.ini");
    INVARIANT(controller_idx < (server_physical_num / 2));
    int ret = 0;

    // NOTE: now we deploy controller in the same physical machine with servers
    // If we use an individual physical machine, we can comment all the setaffinity statements
    CPU_ZERO(&nonserverworker_cpuset);
    for (int i = server_worker_corenums[0]; i < server_total_corenums[0]; i++) {
        CPU_SET(i, &nonserverworker_cpuset);
    }
    // ret = sched_setaffinity(0, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
    pthread_t main_thread = pthread_self();
    ret = pthread_setaffinity_np(main_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
    if (ret) {
        printf("[Error] fail to set affinity of controller.main; errno: %d\n", errno);
        exit(-1);
    }

    prepare_controller();

    pthread_t popserver_thread;
    ret = pthread_create(&popserver_thread, nullptr, run_controller_popserver, nullptr);
    if (ret) {
        COUT_N_EXIT("Error: " << ret);
    }
    ret = pthread_setaffinity_np(popserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
    if (ret) {
        printf("Error of setaffinity for controller.popserver; errno: %d\n", errno);
        exit(-1);
    }

    pthread_t evictserver_thread;
    ret = pthread_create(&evictserver_thread, nullptr, run_controller_evictserver, nullptr);
    if (ret) {
        COUT_N_EXIT("Error: " << ret);
    }
    ret = pthread_setaffinity_np(evictserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
    if (ret) {
        printf("Error of setaffinity for controller.evictserver; errno: %d\n", errno);
        exit(-1);
    }

    while (controller_ready_threads < controller_expected_ready_threads)
        sleep(1);
    printf("[controller] all threads ready\n");
    fflush(stdout);

    controller_running = true;

    // connections from servers
    while (!is_controller_popserver_finish)
        sleep(1);
    printf("[controller] controller.popserver finishes\n");

    controller_running = false;

    void* status;
    int rc = pthread_join(popserver_thread, &status);
    if (rc) {
        COUT_N_EXIT("Error:unable to join popserver " << rc);
    }
    rc = pthread_join(evictserver_thread, &status);
    if (rc) {
        COUT_N_EXIT("Error:unable to join," << rc);
    }

    free_common();
    close_controller();
    printf("[controller] all threads end\n");
}

void prepare_controller() {
    printf("[controller] prepare start\n");

    controller_running = false;

    controller_expected_ready_threads = 2;

    // prepare popserver sockets
    prepare_udpserver(controller_popserver_udpsock, false, controller_popserver_port_start, "controller.popserver");
    create_udpsock(controller_popserver_popclient_udpsock, true, "controller.popserver.popclient");

    // prepare evictserver
    prepare_udpserver(controller_evictserver_udpsock, false, controller_evictserver_port, "controller.evictserver");

    // prepare evictclients
    /*controller_evictserver_evictclient_tcpsock_list = new int[server_num];
    for (size_t i = 0; i < server_num; i++) {
            create_tcpsock(controller_evictserver_evictclient_tcpsock_list[i], false, "controller.evictserver.evictclient");
    }*/

    // prepare evictclient
    create_udpsock(controller_evictserver_evictclient_udpsock, true, "controller.evictserver.evictclient");

    memory_fence();

    printf("[controller] prepare end\n");
}

void* run_controller_popserver(void* param) {
    struct sockaddr_in switchos_popworker_addr;
    socklen_t switchos_popworker_addrlen = sizeof(struct sockaddr);

    printf("[controller.popserver] ready\n");
    controller_ready_threads++;

    while (!controller_running) {
    }

    // Process NETCACHE_CACHE_POP packet <optype, key, serveridx>
    char buf[MAX_BUFSIZE];
    int recvsize = 0;
    bool is_timeout = false;
    while (controller_running) {
        udprecvfrom(controller_popserver_udpsock, buf, MAX_BUFSIZE, 0, &switchos_popworker_addr, &switchos_popworker_addrlen, recvsize, "controller.popserver");

        // printf("receive NETCACHE_CACHE_POP or NETCACHE_CACHE_POP_FINISH from switchos and send it to server\n");
        // dump_buf(buf, recvsize);
        packet_type_t tmp_optype = get_packet_type(buf, recvsize);
        netreach_key_t tmp_key;
        uint16_t tmp_serveridx = 0;
        if (tmp_optype == packet_type_t::NETCACHE_CACHE_POP) {
            netcache_cache_pop_t tmp_netcache_cache_pop(CURMETHOD_ID, buf, recvsize);
            tmp_key = tmp_netcache_cache_pop.key();
            tmp_serveridx = tmp_netcache_cache_pop.serveridx();
        } else if (tmp_optype == packet_type_t::NETCACHE_CACHE_POP_FINISH) {
            netcache_cache_pop_finish_t tmp_netcache_cache_pop_finish(CURMETHOD_ID, buf, recvsize);
            tmp_key = tmp_netcache_cache_pop_finish.key();
            tmp_serveridx = tmp_netcache_cache_pop_finish.serveridx();
        } else {
            printf("[controller.popserver] invalid packet type: %x\n", optype_t(tmp_optype));
            exit(-1);
        }
        INVARIANT(tmp_serveridx >= 0 && tmp_serveridx < max_server_total_logical_num);

        // find corresponding server X
        int tmp_server_physical_idx = -1;
        for (int i = 0; i < server_physical_num; i++) {
            for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
                if (tmp_serveridx == server_logical_idxes_list[i][j]) {
                    tmp_server_physical_idx = i;
                    break;
                }
            }
        }
        INVARIANT(tmp_server_physical_idx != -1);

        // controller.popserver.popclient <-> server.popserver X
        struct sockaddr_in tmp_server_popserver_addr;
        set_sockaddr(tmp_server_popserver_addr, inet_addr(server_ip_for_controller_list[tmp_server_physical_idx]), server_popserver_port_start + tmp_serveridx);
        socklen_t tmp_server_popserver_addrlen = sizeof(struct sockaddr);

        // send NETCACHE_CACHE_POP/_FINISH to corresponding server
        udpsendto(controller_popserver_popclient_udpsock, buf, recvsize, 0, &tmp_server_popserver_addr, tmp_server_popserver_addrlen, "controller.popserver.popclient");

        // receive NETCACHE_CACHE_POP_ACK/_FINISH_ACK from corresponding server
        bool is_timeout = udprecvfrom(controller_popserver_popclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient");
        if (!is_timeout) {  // NOTE: if timeout, it will be covered by switcho.popworker
            // send NETCACHE_CACHE_POP_ACK/_FINISH_ACK to switchos.popworker immediately to avoid timeout
            packet_type_t tmp_acktype = get_packet_type(buf, recvsize);
            if (tmp_acktype == packet_type_t::NETCACHE_CACHE_POP_ACK) {
                netcache_cache_pop_ack_t tmp_netcache_cache_pop_ack(CURMETHOD_ID, buf, recvsize);
                INVARIANT(tmp_netcache_cache_pop_ack.key() == tmp_key);
            } else if (tmp_acktype == packet_type_t::NETCACHE_CACHE_POP_ACK_NLATEST) {
                netcache_cache_pop_ack_nlatest_t tmp_netcache_cache_pop_ack_nlatest(CURMETHOD_ID, buf, recvsize);
                INVARIANT(tmp_netcache_cache_pop_ack_nlatest.key() == tmp_key);
            } else if (tmp_acktype == packet_type_t::NETCACHE_CACHE_POP_FINISH_ACK) {
                netcache_cache_pop_finish_ack_t tmp_netcache_cache_pop_finish_ack(CURMETHOD_ID, buf, recvsize);
                INVARIANT(tmp_netcache_cache_pop_finish_ack.key() == tmp_key);
            } else {
                printf("[controller.popserver] invalid acktype: %x\n", optype_t(tmp_acktype));
                exit(-1);
            }
            udpsendto(controller_popserver_udpsock, buf, recvsize, 0, &switchos_popworker_addr, switchos_popworker_addrlen, "controller.popserver");
        }
    }

    is_controller_popserver_finish = true;
    close(controller_popserver_udpsock);
    close(controller_popserver_popclient_udpsock);
    pthread_exit(nullptr);
}

void* run_controller_evictserver(void* param) {
    struct sockaddr_in switchos_evictclient_addr;
    unsigned int switchos_evictclient_addrlen = sizeof(struct sockaddr);
    // bool with_switchos_evictclient_addr = false;

    struct sockaddr_in server_evictserver_addr;
    memset(&server_evictserver_addr, 0, sizeof(server_evictserver_addr));
    // set_sockaddr(server_evictserver_addr, inet_addr(server_ip_for_controller), server_evictserver_port_start);
    socklen_t server_evictserver_addrlen = sizeof(struct sockaddr_in);

    printf("[controller.evictserver] ready\n");
    controller_ready_threads++;

    while (!controller_running) {
    }

    // process NETCACHE_CACHE_EVICT packet <optype, key, serveridx>
    char buf[MAX_BUFSIZE];
    int recvsize = 0;
    while (controller_running) {
        /*if (!with_switchos_evictclient_addr) {
                udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, &switchos_evictclient_addr, &switchos_evictclient_addrlen, recvsize, "controller.evictserver");
                with_switchos_evictclient_addr = true;
        }
        else {
                udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver");
        }*/
        udprecvfrom(controller_evictserver_udpsock, buf, MAX_BUFSIZE, 0, &switchos_evictclient_addr, &switchos_evictclient_addrlen, recvsize, "controller.evictserver");

        // set dstaddr for the corresponding server
        netcache_cache_evict_t tmp_netcache_cache_evict(CURMETHOD_ID, buf, recvsize);
        uint16_t tmp_global_server_logical_idx = tmp_netcache_cache_evict.serveridx();
        INVARIANT(tmp_global_server_logical_idx >= 0 && tmp_global_server_logical_idx < max_server_total_logical_num);
        int tmp_server_physical_idx = -1;
        for (int i = 0; i < server_physical_num; i++) {
            for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
                if (tmp_global_server_logical_idx == server_logical_idxes_list[i][j]) {
                    tmp_server_physical_idx = i;
                    break;
                }
            }
        }
        INVARIANT(tmp_server_physical_idx != -1);

        memset(&server_evictserver_addr, 0, sizeof(server_evictserver_addr));
        set_sockaddr(server_evictserver_addr, inet_addr(server_ip_for_controller_list[tmp_server_physical_idx]), server_evictserver_port_start + tmp_global_server_logical_idx);

        // printf("receive NETCACHE_CACHE_EVICT from switchos and send to server\n");
        // dump_buf(buf, recvsize);
        udpsendto(controller_evictserver_evictclient_udpsock, buf, recvsize, 0, &server_evictserver_addr, server_evictserver_addrlen, "controller.evictserver.evictclient");

        // NOTE: timeout-and-retry of NETCACHE_CACHE_EVICT is handled by switchos.popworker.evictclient (cover entire eviction workflow)
        bool is_timeout = udprecvfrom(controller_evictserver_evictclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver.evictclient");
        if (!is_timeout) {
            // send NETCACHE_CACHE_EVICT_ACK to switchos.popworker.evictclient
            // printf("receive NETCACHE_CACHE_EVICT_ACK from server and send to switchos\n");
            // dump_buf(buf, recvsize);
            netcache_cache_evict_ack_t tmp_netcache_cache_evict_ack(CURMETHOD_ID, buf, recvsize);
            INVARIANT(tmp_netcache_cache_evict_ack.key() == tmp_netcache_cache_evict.key());
            udpsendto(controller_evictserver_udpsock, buf, recvsize, 0, &switchos_evictclient_addr, switchos_evictclient_addrlen, "controller.evictserver");
        }
    }

    close(controller_evictserver_udpsock);
    close(controller_evictserver_evictclient_udpsock);
    pthread_exit(nullptr);
}

void close_controller() {
    return;
}
