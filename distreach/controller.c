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

struct SnapshotclientSubthreadParam {
    int udpsock;
    struct sockaddr_in dstaddr;
    socklen_t dstaddrlen;
    uint16_t global_server_logical_idx;
};
typedef SnapshotclientSubthreadParam snapshotclient_subthread_param_t;

bool volatile controller_running = false;
std::atomic<size_t> controller_ready_threads(0);
size_t controller_expected_ready_threads = -1;

// cache population/eviction

// server.popclient <-> controller.popserver
int* controller_popserver_udpsock_list = NULL;
// controller.popclient <-> switchos.popserver
int* controller_popserver_popclient_udpsock_list = NULL;
// TODO: replace with SIGTERM
std::atomic<size_t> controller_popserver_finish_threads(0);
size_t controller_expected_popserver_finish_threads = -1;

// switchos.popworker <-> controller.evictserver
int controller_evictserver_udpsock = -1;
// controller.evictclients <-> servers.evictservers
// NOTE: evictclient.index = serveridx
// bool volatile is_controller_evictserver_evictclients_connected = false;
// int * controller_evictserver_evictclient_tcpsock_list = NULL;
// controller.evictclient <-> server.evictserver
int controller_evictserver_evictclient_udpsock = -1;

// snapshot

int controller_snapshotid = 1;  // server uses snapshot id 0 after loading phase

// controller.snapshotclient <-> switchos/per-server.snapshotserver
int controller_snapshotclient_for_switchos_udpsock = -1;
int* controller_snapshotclient_for_server_udpsock_list = NULL;
// written by controller.snapshotclient; read by controller.snapshotclient.senddata_subthread to server.snapshotdataserver
dynamic_array_t* controller_snapshotclient_for_server_databuf_list = NULL;

// control plane bandwidth usage
std::atomic<uint64_t> bandwidthcost(0);            // in unit of byte (cleared per snapshot period); including local control plane BW cost; accessed by snapshotclient/popserver/evictserver
uint64_t* perserver_localcp_bandwidthcost = NULL;  // in unit of byte; only for local control plane BW cost (speical cases); accessed only by snapshotclient

void prepare_controller();
void* run_controller_popserver(void* param);                // Receive CACHE_POPs from each server
void* run_controller_evictserver(void* param);              // Forward CACHE_EVICT to server and CACHE_EVICT_ACK to switchos in cache eviction
void controller_load_snapshotid();                          // retrieve latest snapshot id
void controller_update_snapshotid(char* buf, int bufsize);  // store latest snapshotid and inswitch snapshot data
void* run_controller_snapshotclient(void* param);           // Periodically notify switch os to launch snapshot
void* run_controller_snapshotclient_cleanup_subthread(void* param);
void* run_controller_snapshotclient_start_subthread(void* param);
void* run_controller_snapshotclient_senddata_subthread(void* param);
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

    pthread_t popserver_threads[max_server_total_logical_num];
    uint16_t popserver_params[max_server_total_logical_num];
    for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_server_total_logical_num; tmp_global_server_logical_idx++) {
        popserver_params[tmp_global_server_logical_idx] = tmp_global_server_logical_idx;
        ret = pthread_create(&popserver_threads[tmp_global_server_logical_idx], nullptr, run_controller_popserver, &popserver_params[tmp_global_server_logical_idx]);
        if (ret) {
            COUT_N_EXIT("Error: " << ret);
        }
        ret = pthread_setaffinity_np(popserver_threads[tmp_global_server_logical_idx], sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
        if (ret) {
            printf("Error of setaffinity for controller.popserver; errno: %d\n", errno);
            exit(-1);
        }
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

    pthread_t snapshotclient_thread;
    ret = pthread_create(&snapshotclient_thread, nullptr, run_controller_snapshotclient, nullptr);
    if (ret) {
        COUT_N_EXIT("Error: " << ret);
    }
    // NOTE: subthreads created by controller.snapshotclient via pthread_create will inherit the CPU affinity mask of snapshotclient_thread
    ret = pthread_setaffinity_np(snapshotclient_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
    if (ret) {
        printf("Error of setaffinity for controller.snapshotclient; errno: %d\n", errno);
        exit(-1);
    }

    while (controller_ready_threads < controller_expected_ready_threads)
        sleep(1);
    printf("[controller] all threads ready\n");
    fflush(stdout);

    controller_running = true;

    // connections from servers
    while (controller_popserver_finish_threads < controller_expected_popserver_finish_threads)
        sleep(1);
    printf("[controller] all popservers finish\n");

    controller_running = false;

    void* status;
    for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_server_total_logical_num; tmp_global_server_logical_idx++) {
        int rc = pthread_join(popserver_threads[tmp_global_server_logical_idx], &status);
        if (rc) {
            COUT_N_EXIT("Error:unable to join popserver " << rc);
        }
    }
    int rc = pthread_join(evictserver_thread, &status);
    if (rc) {
        COUT_N_EXIT("Error:unable to join," << rc);
    }
    rc = pthread_join(snapshotclient_thread, &status);
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

    controller_expected_ready_threads = max_server_total_logical_num + 2;
    controller_expected_popserver_finish_threads = max_server_total_logical_num;

    // prepare popserver sockets
    controller_popserver_udpsock_list = new int[max_server_total_logical_num];
    controller_popserver_popclient_udpsock_list = new int[max_server_total_logical_num];
    for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_server_total_logical_num; tmp_global_server_logical_idx++) {
        prepare_udpserver(controller_popserver_udpsock_list[tmp_global_server_logical_idx], false, controller_popserver_port_start + tmp_global_server_logical_idx, "controller.popserver");
        create_udpsock(controller_popserver_popclient_udpsock_list[tmp_global_server_logical_idx], true, "controller.popserver.popclient");
    }

    // controller_cachedkey_serveridx_map.clear();
    /*controller_cache_pop_ptrs = new cache_pop_t*[MQ_SIZE];
    for (size_t i = 0; i < MQ_SIZE; i++) {
            controller_cache_pop_ptrs[i] = NULL;
    }
    controller_head_for_pop = 0;
    controller_tail_for_pop = 0;*/

    // prepare evictserver
    prepare_udpserver(controller_evictserver_udpsock, false, controller_evictserver_port, "controller.evictserver");

    // prepare evictclients
    /*controller_evictserver_evictclient_tcpsock_list = new int[server_num];
    for (size_t i = 0; i < server_num; i++) {
            create_tcpsock(controller_evictserver_evictclient_tcpsock_list[i], false, "controller.evictserver.evictclient");
    }*/

    // prepare evictclient
    create_udpsock(controller_evictserver_evictclient_udpsock, true, "controller.evictserver.evictclient");

    // load latest snapshotid
    controller_load_snapshotid();

    // prepare snapshotclient
    create_udpsock(controller_snapshotclient_for_switchos_udpsock, true, "controller.snapshotclient_for_switchos", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
    controller_snapshotclient_for_server_udpsock_list = new int[max_server_total_logical_num];
    controller_snapshotclient_for_server_databuf_list = new dynamic_array_t[max_server_total_logical_num];
    for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_server_total_logical_num; tmp_global_server_logical_idx++) {
        create_udpsock(controller_snapshotclient_for_server_udpsock_list[tmp_global_server_logical_idx], true, "controller.snapshotclient_for_server", SOCKET_TIMEOUT, 0, UDP_LARGE_RCVBUFSIZE);
        controller_snapshotclient_for_server_databuf_list[tmp_global_server_logical_idx].init(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
    }

    perserver_localcp_bandwidthcost = new uint64_t[max_server_total_logical_num];
    memset(perserver_localcp_bandwidthcost, 0, sizeof(uint64_t) * max_server_total_logical_num);

    memory_fence();

    printf("[controller] prepare end\n");
}

void controller_load_snapshotid() {
    std::string snapshotid_path;
    get_controller_snapshotid_path(CURMETHOD_ID, snapshotid_path);
    if (isexist(snapshotid_path)) {
        load_snapshotid(controller_snapshotid, snapshotid_path);
        controller_snapshotid += 1;
    } else {
        controller_snapshotid = 1;
    }
    return;
}

void controller_update_snapshotid(char* buf, int bufsize) {
    // TODO: store inswitch snapshot data for switch failure
    std::string snapshotdata_path;
    get_controller_snapshotdata_path(CURMETHOD_ID, snapshotdata_path, controller_snapshotid);
    store_buf(buf, bufsize, snapshotdata_path);
    // store latest snapshot id for controller failure
    std::string snapshotid_path;
    get_controller_snapshotid_path(CURMETHOD_ID, snapshotid_path);
    store_snapshotid(controller_snapshotid, snapshotid_path);
    // remove old-enough snapshot data
    int old_snapshotid = controller_snapshotid - 1;
    if (old_snapshotid > 0) {
        std::string old_snapshotdata_path;
        get_controller_snapshotdata_path(CURMETHOD_ID, old_snapshotdata_path, old_snapshotid);
        rmfiles(old_snapshotdata_path.c_str());
    }

    controller_snapshotid += 1;
}

void* run_controller_popserver(void* param) {
    // controlelr.popserver i <-> server.popclient i
    uint16_t global_server_logical_idx = *((uint16_t*)param);

    struct sockaddr_in server_popclient_addr;
    socklen_t server_popclient_addrlen = sizeof(struct sockaddr);
    // bool with_server_popclient_addr = false;

    // controller.popserver.popclient i <-> switchos.popserver
    struct sockaddr_in switchos_popserver_addr;
    set_sockaddr(switchos_popserver_addr, inet_addr(switchos_ip), switchos_popserver_port);
    socklen_t switchos_popserver_addrlen = sizeof(struct sockaddr);

    printf("[controller.popserver %d] ready\n", global_server_logical_idx);
    controller_ready_threads++;

    while (!controller_running) {
    }

    // Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>
    char buf[MAX_BUFSIZE];
    int recvsize = 0;
    bool is_timeout = false;
    while (controller_running) {
        /*if (!with_server_popclient_addr) {
                udprecvfrom(controller_popserver_udpsock_list[idx], buf, MAX_BUFSIZE, 0, &server_popclient_addr, &server_popclient_addrlen, recvsize, "controller.popserver");
                with_server_popclient_addr = true;
        }
        else {
                udprecvfrom(controller_popserver_udpsock_list[idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver");
        }*/
        udprecvfrom(controller_popserver_udpsock_list[global_server_logical_idx], buf, MAX_BUFSIZE, 0, &server_popclient_addr, &server_popclient_addrlen, recvsize, "controller.popserver");

        // printf("receive CACHE_POP from server and send it to switchos\n");
        // dump_buf(buf, recvsize);
        cache_pop_t tmp_cache_pop(CURMETHOD_ID, buf, recvsize);

        // update bandwidth usage
        bandwidthcost += tmp_cache_pop.bwcost();

        // send CACHE_POP to switch os
        udpsendto(controller_popserver_popclient_udpsock_list[global_server_logical_idx], buf, recvsize, 0, &switchos_popserver_addr, switchos_popserver_addrlen, "controller.popserver.popclient");

        // receive CACHE_POP_ACK from switch os
        bool is_timeout = udprecvfrom(controller_popserver_popclient_udpsock_list[global_server_logical_idx], buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.popserver.popclient");
        if (!is_timeout) {
            // send CACHE_POP_ACK to server.popclient immediately to avoid timeout
            cache_pop_ack_t tmp_cache_pop_ack(CURMETHOD_ID, buf, recvsize);
            INVARIANT(tmp_cache_pop_ack.key() == tmp_cache_pop.key());
            udpsendto(controller_popserver_udpsock_list[global_server_logical_idx], buf, recvsize, 0, &server_popclient_addr, server_popclient_addrlen, "controller.popserver");

            // update bandwidth usage
            bandwidthcost += tmp_cache_pop_ack.bwcost();
        }

        /*if (controller_cachedkey_serveridx_map.find(tmp_cache_pop->key()) == controller_cachedkey_serveridx_map.end()) {
                controller_cachedkey_serveridx_map.insert(std::pair<netreach_key_t, uint32_t>(tmp_cache_pop_ptr->key(), tmp_cache_pop_ptr->serveridx()));
        }
        else {
                printf("[controller] Receive duplicate key from server %ld!", tmp_cache_pop_ptr->serveridx());
                exit(-1);
        }*/
    }

    controller_popserver_finish_threads++;
    close(controller_popserver_udpsock_list[global_server_logical_idx]);
    close(controller_popserver_popclient_udpsock_list[global_server_logical_idx]);
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

    // process CACHE_EVICT/_CASE2 packet <optype, key, vallen, value, result, seq, serveridx>
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
        cache_evict_t* tmp_cache_evict_ptr;
        packet_type_t optype = get_packet_type(buf, recvsize);
        if (optype == packet_type_t::CACHE_EVICT) {
            tmp_cache_evict_ptr = new cache_evict_t(CURMETHOD_ID, buf, recvsize);
        } else if (optype == packet_type_t::CACHE_EVICT_CASE2) {
            tmp_cache_evict_ptr = new cache_evict_case2_t(CURMETHOD_ID, buf, recvsize);
        }

        // update bandwidth usage
        bandwidthcost += tmp_cache_evict_ptr->bwcost();

        // verify serveridx
        uint16_t tmp_global_server_logical_idx = tmp_cache_evict_ptr->serveridx();
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
        delete tmp_cache_evict_ptr;
        tmp_cache_evict_ptr = NULL;

        // printf("receive CACHE_EVICT from switchos and send to server\n");
        // dump_buf(buf, recvsize);
        udpsendto(controller_evictserver_evictclient_udpsock, buf, recvsize, 0, &server_evictserver_addr, server_evictserver_addrlen, "controller.evictserver.evictclient");

        // NOTE: timeout-and-retry of CACHE_EVICT is handled by switchos.popworker.evictclient (cover entire eviction workflow)
        bool is_timeout = udprecvfrom(controller_evictserver_evictclient_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.evictserver.evictclient");
        if (!is_timeout) {
            // update bandwidth usage
            cache_evict_ack_t tmpack(CURMETHOD_ID, buf, recvsize);
            bandwidthcost += tmpack.bwcost();

            // send CACHE_EVICT_ACK to switchos.popworker.evictclient
            // printf("receive CACHE_EVICT_ACK from server and send to switchos\n");
            // dump_buf(buf, recvsize);
            udpsendto(controller_evictserver_udpsock, buf, recvsize, 0, &switchos_evictclient_addr, switchos_evictclient_addrlen, "controller.evictserver");
        }
    }

    close(controller_evictserver_udpsock);
    close(controller_evictserver_evictclient_udpsock);
    pthread_exit(nullptr);
}

void* run_controller_snapshotclient(void* param) {
    // get valid server logical idxes
    std::vector<uint16_t> valid_global_server_logical_idxes;
    for (int i = 0; i < server_physical_num; i++) {
        for (int j = 0; j < server_logical_idxes_list[i].size(); j++) {
            uint16_t tmp_global_server_logical_idx = server_logical_idxes_list[i][j];
            valid_global_server_logical_idxes.push_back(tmp_global_server_logical_idx);
        }
    }

    struct sockaddr_in switchos_snapshotserver_addr;
    set_sockaddr(switchos_snapshotserver_addr, inet_addr(switchos_ip), switchos_snapshotserver_port);
    socklen_t switchos_snapshotserver_addrlen = sizeof(struct sockaddr_in);

    struct sockaddr_in server_snapshotserver_addr_list[max_server_total_logical_num];
    socklen_t server_snapshotserver_addrlen_list[max_server_total_logical_num];
    struct sockaddr_in server_snapshotdataserver_addr_list[max_server_total_logical_num];
    socklen_t server_snapshotdataserver_addrlen_list[max_server_total_logical_num];
    // for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_server_total_logical_num; tmp_global_server_logical_idx++) {
    for (int valid_idx = 0; valid_idx < valid_global_server_logical_idxes.size(); valid_idx++) {
        uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[valid_idx];
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
        const char* tmp_dstip = server_ip_for_controller_list[tmp_server_physical_idx];
        /*if (strcmp(server_ip_for_controller_list[tmp_server_physical_idx], controller_ip_for_server) == 0) {
                tmp_dstip = "127.0.0.1";
        }*/
        set_sockaddr(server_snapshotserver_addr_list[tmp_global_server_logical_idx], inet_addr(tmp_dstip), server_snapshotserver_port_start + tmp_global_server_logical_idx);
        server_snapshotserver_addrlen_list[tmp_global_server_logical_idx] = sizeof(struct sockaddr_in);
        set_sockaddr(server_snapshotdataserver_addr_list[tmp_global_server_logical_idx], inet_addr(tmp_dstip), server_snapshotdataserver_port_start + tmp_global_server_logical_idx);
        server_snapshotdataserver_addrlen_list[tmp_global_server_logical_idx] = sizeof(struct sockaddr_in);
    }

    // prepare for concurrent SNAPSHOT_CLEANUP (param.serveridx not used)
    pthread_t cleanup_subthread_for_switchos;
    snapshotclient_subthread_param_t cleanup_subthread_param_for_switchos;
    cleanup_subthread_param_for_switchos.udpsock = controller_snapshotclient_for_switchos_udpsock;
    cleanup_subthread_param_for_switchos.dstaddr = switchos_snapshotserver_addr;
    cleanup_subthread_param_for_switchos.dstaddrlen = switchos_snapshotserver_addrlen;
    pthread_t cleanup_subthread_for_server_list[max_server_total_logical_num];
    snapshotclient_subthread_param_t cleanup_subthread_param_for_server_list[max_server_total_logical_num];
    for (int valid_idx = 0; valid_idx < valid_global_server_logical_idxes.size(); valid_idx++) {
        uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[valid_idx];
        cleanup_subthread_param_for_server_list[tmp_global_server_logical_idx].udpsock = controller_snapshotclient_for_server_udpsock_list[tmp_global_server_logical_idx];
        cleanup_subthread_param_for_server_list[tmp_global_server_logical_idx].dstaddr = server_snapshotserver_addr_list[tmp_global_server_logical_idx];
        cleanup_subthread_param_for_server_list[tmp_global_server_logical_idx].dstaddrlen = server_snapshotserver_addrlen_list[tmp_global_server_logical_idx];
        cleanup_subthread_param_for_server_list[tmp_global_server_logical_idx].global_server_logical_idx = tmp_global_server_logical_idx;
    }

    // prepare for concurrent SNAPSHOT_START (param.serveridx not used)
    pthread_t start_subthread_for_switchos;
    snapshotclient_subthread_param_t start_subthread_param_for_switchos;
    start_subthread_param_for_switchos.udpsock = controller_snapshotclient_for_switchos_udpsock;
    start_subthread_param_for_switchos.dstaddr = switchos_snapshotserver_addr;
    start_subthread_param_for_switchos.dstaddrlen = switchos_snapshotserver_addrlen;
    pthread_t start_subthread_for_server_list[max_server_total_logical_num];
    snapshotclient_subthread_param_t start_subthread_param_for_server_list[max_server_total_logical_num];
    for (int valid_idx = 0; valid_idx < valid_global_server_logical_idxes.size(); valid_idx++) {
        uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[valid_idx];
        start_subthread_param_for_server_list[tmp_global_server_logical_idx].udpsock = controller_snapshotclient_for_server_udpsock_list[tmp_global_server_logical_idx];
        start_subthread_param_for_server_list[tmp_global_server_logical_idx].dstaddr = server_snapshotserver_addr_list[tmp_global_server_logical_idx];
        start_subthread_param_for_server_list[tmp_global_server_logical_idx].dstaddrlen = server_snapshotserver_addrlen_list[tmp_global_server_logical_idx];
        start_subthread_param_for_server_list[tmp_global_server_logical_idx].global_server_logical_idx = tmp_global_server_logical_idx;
    }

    // prepare for concurrent SNAPSHOT_SENDDATA
    pthread_t senddata_subthread_for_server_list[max_server_total_logical_num];
    snapshotclient_subthread_param_t senddata_subthread_param_for_server_list[max_server_total_logical_num];
    for (int valid_idx = 0; valid_idx < valid_global_server_logical_idxes.size(); valid_idx++) {
        uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[valid_idx];
        senddata_subthread_param_for_server_list[tmp_global_server_logical_idx].udpsock = controller_snapshotclient_for_server_udpsock_list[tmp_global_server_logical_idx];
        senddata_subthread_param_for_server_list[tmp_global_server_logical_idx].dstaddr = server_snapshotdataserver_addr_list[tmp_global_server_logical_idx];
        senddata_subthread_param_for_server_list[tmp_global_server_logical_idx].dstaddrlen = server_snapshotdataserver_addrlen_list[tmp_global_server_logical_idx];
        senddata_subthread_param_for_server_list[tmp_global_server_logical_idx].global_server_logical_idx = tmp_global_server_logical_idx;
    }

    // prepare for SNAPSHOT_GETDATA_ACK
    dynamic_array_t databuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);

    // prepare for UPSTREAM_BACKUP_NOTIFICATION
    dynamic_array_t notificationbuf(MAX_BUFSIZE, MAX_LARGE_BUFSIZE);
    int controller_snapshotclient_for_upstreamnotification_udpsocks[client_physical_num];
    struct sockaddr_in client_backupreleaser_addr_list[client_physical_num];
    socklen_t client_backupreleaser_addrlen_list[client_physical_num];
    for (int i = 0; i < client_physical_num; i++) {
        create_udpsock(controller_snapshotclient_for_upstreamnotification_udpsocks[i], false, "controller.snapshotclient_for_upstreamnotification");
        const char* tmp_clientip = client_ip_for_client0_list[i];
        set_sockaddr(client_backupreleaser_addr_list[i], inet_addr(tmp_clientip), client_upstreambackupreleaser_port);
        client_backupreleaser_addrlen_list[i] = sizeof(struct sockaddr_in);
    }

    printf("[controller.snapshotclient] ready\n");
    controller_ready_threads++;

    while (!controller_running) {
    }

    printf("Wait for notification of preparefinishclient...\n");
    int warmupfinishserver_udpsock = -1;
    prepare_udpserver(warmupfinishserver_udpsock, false, controller_warmupfinishserver_port, "controller.warmupfinishserver");
    char warmupfinishbuf[256];
    int warmupfinish_recvsize = 0;
    udprecvfrom(warmupfinishserver_udpsock, warmupfinishbuf, 256, 0, NULL, NULL, warmupfinish_recvsize, "controller.warmupfinishserver");
    close(warmupfinishserver_udpsock);

    bool is_snapshot_enabled = false;
    if (controller_snapshot_period > 0) {
        is_snapshot_enabled = true;
        printf("[controller.snapshotclient] Start periodic snapshot with period of %d ms...\n", controller_snapshot_period);
        fflush(stdout);
    } else {
        is_snapshot_enabled = false;
        printf("[controller.snapshotclient] Disable periodic snapshot due to period of 0 -> dump per-second bandwidth...\n");
        fflush(stdout);
    }

    if (is_snapshot_enabled) {
        char sendbuf[MAX_BUFSIZE];
        int sendsize = 0;
        char recvbuf[MAX_BUFSIZE];
        int recvsize = 0;
        bool is_timeout = false;
        struct timespec snapshot_t1, snapshot_t2, snapshot_t3;
        while (controller_running) {
            // TMPDEBUG
            // printf("Type to send SNAPSHOT_START...\n");
            // getchar();

            CUR_TIME(snapshot_t1);

            printf("Snapshotid: %d\n", controller_snapshotid);
            fflush(stdout);

            // (1) send SNAPSHOT_CLEANUP to each switchos and server concurrently
            printf("[controller.snapshotclient] send SNAPSHOT_CLEANUPs to each switchos and server\n");
            fflush(stdout);
            pthread_create(&cleanup_subthread_for_switchos, nullptr, run_controller_snapshotclient_cleanup_subthread, &cleanup_subthread_param_for_switchos);
            for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[i];
                pthread_create(&cleanup_subthread_for_server_list[tmp_global_server_logical_idx], nullptr, run_controller_snapshotclient_cleanup_subthread, &cleanup_subthread_param_for_server_list[tmp_global_server_logical_idx]);
            }
            pthread_join(cleanup_subthread_for_switchos, NULL);
            for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[i];
                pthread_join(cleanup_subthread_for_server_list[tmp_global_server_logical_idx], NULL);
            }
            bandwidthcost += (valid_global_server_logical_idxes.size() + 1) * 2 * sizeof(int);  // update bandwidth usage

            // (2) send SNAPSHOT_PREPARE to each switch os to enable single path for snapshot atomicity
            printf("[controller.snapshotclient] send SNAPSHOT_PREPARE to each switchos\n");
            fflush(stdout);
            snapshot_signal_t snapshot_prepare_signal(SNAPSHOT_PREPARE, controller_snapshotid);
            sendsize = snapshot_prepare_signal.serialize(sendbuf, MAX_BUFSIZE);
            // memcpy(sendbuf, &SNAPSHOT_PREPARE, sizeof(int));
            // memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));
            while (true) {
                udpsendto(controller_snapshotclient_for_switchos_udpsock, sendbuf, sendsize, 0, &switchos_snapshotserver_addr, switchos_snapshotserver_addrlen, "controller.snapshotclient");

                is_timeout = udprecvfrom(controller_snapshotclient_for_switchos_udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient");
                if (is_timeout) {
                    continue;
                } else {
                    snapshot_signal_t prepareack(recvbuf, recvsize);
                    INVARIANT(prepareack.control_type() == SNAPSHOT_PREPARE_ACK);
                    // INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_PREPARE_ACK);
                    break;
                }
            }
            bandwidthcost += 2 * sizeof(int);  // update bandwidth usage

#ifdef DEBUG_SNAPSHOT
            // TMPDEBUG
            printf("Type to set snapshot flag...\n");
            getchar();
#endif

            // (3) send SNAPSHOT_SETFLAG to each switch os to set snapshot flag
            printf("[controller.snapshotclient] send SNAPSHOT_SETFLAG to each switchos\n");
            fflush(stdout);
            snapshot_signal_t snapshot_setflag_signal(SNAPSHOT_SETFLAG, controller_snapshotid);
            sendsize = snapshot_setflag_signal.serialize(sendbuf, MAX_BUFSIZE);
            // memcpy(sendbuf, &SNAPSHOT_SETFLAG, sizeof(int));
            // memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));
            while (true) {
                udpsendto(controller_snapshotclient_for_switchos_udpsock, sendbuf, sendsize, 0, &switchos_snapshotserver_addr, switchos_snapshotserver_addrlen, "controller.snapshotclient");

                is_timeout = udprecvfrom(controller_snapshotclient_for_switchos_udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient");
                if (is_timeout) {
                    continue;
                } else {
                    snapshot_signal_t setflagack(recvbuf, recvsize);
                    INVARIANT(setflagack.control_type() == SNAPSHOT_SETFLAG_ACK);
                    // INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_SETFLAG_ACK);
                    break;
                }
            }
            bandwidthcost += 2 * sizeof(int);  // update bandwidth usage

            // (4) send SNAPSHOT_START to each switchos and server concurrently
            printf("[controller.snapshotclient] send SNAPSHOT_STARTs to each switchos and server\n");
            fflush(stdout);
            pthread_create(&start_subthread_for_switchos, nullptr, run_controller_snapshotclient_start_subthread, &start_subthread_param_for_switchos);
            for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[i];
                pthread_create(&start_subthread_for_server_list[tmp_global_server_logical_idx], nullptr, run_controller_snapshotclient_start_subthread, &start_subthread_param_for_server_list[tmp_global_server_logical_idx]);
            }
            pthread_join(start_subthread_for_switchos, NULL);
            for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[i];
                pthread_join(start_subthread_for_server_list[tmp_global_server_logical_idx], NULL);
            }
            bandwidthcost += (valid_global_server_logical_idxes.size() + 1) * 2 * sizeof(int);  // update bandwidth usage

            // prepare dynmic arrays for per-server snapshot data
            for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[i];
                controller_snapshotclient_for_server_databuf_list[tmp_global_server_logical_idx].clear();
            }

            // (5) send SNAPSHOT_GETDATA to each switch os to get consistent snapshot data
            printf("[controller.snapshotclient] send SNAPSHOT_GETDATA to each switchos\n");
            fflush(stdout);
            snapshot_signal_t snapshot_getdata_signal(SNAPSHOT_GETDATA, controller_snapshotid);
            sendsize = snapshot_getdata_signal.serialize(sendbuf, MAX_BUFSIZE);
            // memcpy(sendbuf, &SNAPSHOT_GETDATA, sizeof(int));
            // memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));
            while (true) {
                udpsendto(controller_snapshotclient_for_switchos_udpsock, sendbuf, sendsize, 0, &switchos_snapshotserver_addr, switchos_snapshotserver_addrlen, "controller.snapshotclient");
                bandwidthcost += 2 * sizeof(int);  // update bandwidth usage

                databuf.clear();
                is_timeout = udprecvlarge_udpfrag(CURMETHOD_ID, controller_snapshotclient_for_switchos_udpsock, databuf, 0, NULL, NULL, "controller.snapshotclient");
                if (is_timeout) {
                    continue;
                } else {
                    int total_bytes = 0;
                    std::vector<int> perserver_bytes;
                    std::vector<uint16_t> perserver_serveridx;
                    std::vector<int> perserver_recordcnt;
                    std::vector<uint64_t> perserver_specialcase_bwcost;
                    std::vector<std::vector<netreach_key_t>> perserver_keyarray;
                    std::vector<std::vector<val_t>> perserver_valarray;
                    std::vector<std::vector<uint32_t>> perserver_seqarray;
                    std::vector<std::vector<bool>> perserver_statarray;

                    deserialize_snapshot_getdata_ack(databuf, SNAPSHOT_GETDATA_ACK, total_bytes, perserver_bytes, perserver_serveridx, perserver_recordcnt, perserver_specialcase_bwcost, perserver_keyarray, perserver_valarray, perserver_seqarray, perserver_statarray);

                    // TMPDEBUG
                    int tmp_truestatcnt = 0;
                    int tmp_nonzerovalcnt = 0;
                    int tmp_totalcnt = 0;
                    for (int i = 0; i < perserver_statarray.size(); i++) {
                        for (int j = 0; j < perserver_statarray[i].size(); j++) {
                            if (perserver_statarray[i][j]) {
                                tmp_truestatcnt += 1;
                            }
                            if (perserver_valarray[i][j].val_length > 0) {
                                tmp_nonzerovalcnt += 1;
                            }
                            tmp_totalcnt += 1;
                        }
                    }
                    // printf("[DEBUG] stat=true cnt: %d, non-zero vallength cnt: %d, total entry cnt: %d\n", tmp_truestatcnt, tmp_nonzerovalcnt, tmp_totalcnt);
                    // fflush(stdout);

                    // update bandwidth usage
                    bandwidthcost += total_bytes;
                    for (int tmp_bwcostidx = 0; tmp_bwcostidx < perserver_specialcase_bwcost.size(); tmp_bwcostidx++) {
                        bandwidthcost += perserver_specialcase_bwcost[tmp_bwcostidx];

                        uint16_t tmp_serveridx = perserver_serveridx[tmp_bwcostidx];
                        perserver_localcp_bandwidthcost[tmp_serveridx] += perserver_specialcase_bwcost[tmp_bwcostidx];
                    }

                    // prepare per-server snapshot data
                    for (int i = 0; i < perserver_serveridx.size(); i++) {
                        uint16_t tmp_serveridx = perserver_serveridx[i];

                        bool is_valid = false;
                        for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                            if (tmp_serveridx == valid_global_server_logical_idxes[i]) {
                                is_valid = true;
                                break;
                            }
                        }
#ifdef SERVER_ROTATION
                        if (is_valid == false) {
                            continue;
                        }
#else
                        INVARIANT(is_valid == true);
#endif

                        dynamic_serialize_snapshot_senddata(controller_snapshotclient_for_server_databuf_list[tmp_serveridx], SNAPSHOT_SENDDATA, controller_snapshotid, tmp_serveridx, perserver_recordcnt[i], perserver_keyarray[i], perserver_valarray[i], perserver_seqarray[i], perserver_statarray[i]);
                    }

                    // send upstream backup notifications to clients and update bandwidth cost
                    notificationbuf.clear();
                    int notificationbytes = dynamic_serialize_upstream_backup_notification(notificationbuf, perserver_keyarray, perserver_seqarray);
                    for (int i = 0; i < client_physical_num; i++) {
                        udpsendlarge_udpfrag(controller_snapshotclient_for_upstreamnotification_udpsocks[i], notificationbuf.array(), notificationbytes, 0, &client_backupreleaser_addr_list[i], client_backupreleaser_addrlen_list[i], "controller.snapshotclient");
                    }
                    printf("upstream backup notification bytes: %f MiB\n", float(notificationbytes * client_physical_num) / 1024.0 / 1024.0);
                    bandwidthcost += notificationbytes * client_physical_num;

                    break;
                }
            }

            // (6) send SNAPSHOT_SENDDATA to each server concurrently
            printf("[controller.snapshotclient] send SNAPSHOT_SENDDATAs to each server\n");
            fflush(stdout);
            for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[i];
                // NOTE: even if databuf_list[i].size() == default_perserverbytes, we still need to notify the server that snapshot is end by SNAPSHOT_SENDDATA
                pthread_create(&senddata_subthread_for_server_list[tmp_global_server_logical_idx], nullptr, run_controller_snapshotclient_senddata_subthread, &senddata_subthread_param_for_server_list[tmp_global_server_logical_idx]);
            }
            for (int i = 0; i < valid_global_server_logical_idxes.size(); i++) {
                uint16_t tmp_global_server_logical_idx = valid_global_server_logical_idxes[i];
                pthread_join(senddata_subthread_for_server_list[tmp_global_server_logical_idx], NULL);
            }
            bandwidthcost += valid_global_server_logical_idxes.size() * 2 * sizeof(int);  // update bandwidth usage

            CUR_TIME(snapshot_t2);
            DELTA_TIME(snapshot_t2, snapshot_t1, snapshot_t3);
            printf("Time of making consistent system snapshot: %f s\n", GET_MICROSECOND(snapshot_t3) / 1000.0 / 1000.0);
            fflush(stdout);

            // NOTE: append bandwidthcost into tmp_controller_bwcost.out
            double bwcost_rate = bandwidthcost / (controller_snapshot_period / 1000.0) / 1024.0 / 1024.0;  // MiB/s
            double localcp_bwcost_rates[max_server_total_logical_num];
            for (int i = 0; i < max_server_total_logical_num; i++) {
                localcp_bwcost_rates[i] = perserver_localcp_bandwidthcost[i] / (controller_snapshot_period / 1000.0) / 1024.0 / 1024.0;  // MiB/s
            }
            char strid[256];
            memset(strid, '\0', 256);
            if (workload_mode == 0) {
                if (server_physical_num == 1) {
                    sprintf(strid, "static%d-%d", server_total_logical_num_for_rotation, server_logical_idxes_list[0][0]);
                } else {
                    sprintf(strid, "static%d-%d-%d", server_total_logical_num_for_rotation, server_logical_idxes_list[0][0], server_logical_idxes_list[1][0]);
                }
            } else {
                sprintf(strid, "dynamic");
            }
            // printf("bandwidth cost: %f MiB/s\n", bwcost_rate);
            // fflush(stdout);

            FILE* tmpfd = fopen("tmp_controller_bwcost.out", "a+");
            // NOTE: totalbwcost means bwcost of both local and global control plane, while localbwcost means bwcost of local control plane
            fprintf(tmpfd, "%s totalbwcost(MiB/s): %f\n", strid, bwcost_rate);
            fprintf(tmpfd, "%s localbwcost(MiB/s):", strid);
            for (int i = 0; i < max_server_total_logical_num; i++) {
                fprintf(tmpfd, " %f", localcp_bwcost_rates[i]);
            }
            fprintf(tmpfd, "\n");
            fflush(tmpfd);
            fclose(tmpfd);

            // clear to count the next period
            bandwidthcost = 0;
            memset(perserver_localcp_bandwidthcost, 0, sizeof(uint64_t) * max_server_total_logical_num);

            // (7) save per-switch SNAPSHOT_GETDATA_ACK (databuf) for controller failure recovery
            controller_update_snapshotid(databuf.array(), databuf.size());
            // #ifdef SERVER_ROTATION
            //		break;
            // #else
            usleep(controller_snapshot_period * 1000);  // ms -> us
                                                        // #endif

            // #endif
        }
    }       // end of is_snapshot_enabled = true
    else {  // disable the snapshot
        int max_dumpcnt = 10;
        if (workload_mode == 1) {
            max_dumpcnt = 75;
        }
        printf("max_dumpcnt of %d for workload_mode %d\n", max_dumpcnt, workload_mode);
        fflush(stdout);

        int tmp_sleep_interval = 1000;  // 1000ms = 1s
        for (int tmp_dumpidx = 0; tmp_dumpidx < max_dumpcnt; tmp_dumpidx++) {
            usleep(tmp_sleep_interval * 1000);  // ms -> us

            // NOTE: append bandwidthcost into tmp_controller_bwcost.out
            double bwcost_rate = bandwidthcost / (tmp_sleep_interval / 1000.0) / 1024.0 / 1024.0;  // MiB/s
            double localcp_bwcost_rates[max_server_total_logical_num];
            for (int i = 0; i < max_server_total_logical_num; i++) {
                localcp_bwcost_rates[i] = perserver_localcp_bandwidthcost[i] / (tmp_sleep_interval / 1000.0) / 1024.0 / 1024.0;  // MiB/s
            }
            char strid[256];
            memset(strid, '\0', 256);
            if (workload_mode == 0) {
                if (server_physical_num == 1) {
                    sprintf(strid, "static%d-%d", server_total_logical_num_for_rotation, server_logical_idxes_list[0][0]);
                } else {
                    sprintf(strid, "static%d-%d-%d", server_total_logical_num_for_rotation, server_logical_idxes_list[0][0], server_logical_idxes_list[1][0]);
                }
            } else {
                sprintf(strid, "dynamic");
            }
            // printf("bandwidth cost: %f MiB/s\n", bwcost_rate);
            // fflush(stdout);

            FILE* tmpfd = fopen("tmp_controller_bwcost.out", "a+");
            // NOTE: totalbwcost means bwcost of both local and global control plane, while localbwcost means bwcost of local control plane
            // NOTE: totalbwcost must be 0 for static pattern when disabling snapshot generation, yet could > 0 for dynamic pattern due to cache admission/eviction
            fprintf(tmpfd, "%s totalbwcost(MiB/s): %f\n", strid, bwcost_rate);
            // NOTE: localbwcost must be 0 for static/dynamic pattern when disabling snapshot generation
            fprintf(tmpfd, "%s localbwcost(MiB/s):", strid);
            for (int i = 0; i < max_server_total_logical_num; i++) {
                fprintf(tmpfd, " %f", localcp_bwcost_rates[i]);
            }
            fprintf(tmpfd, "\n");
            fflush(tmpfd);
            fclose(tmpfd);

            // clear to count the next period
            bandwidthcost = 0;
            memset(perserver_localcp_bandwidthcost, 0, sizeof(uint64_t) * max_server_total_logical_num);
        }
    }  // end of is_snapshot_enabled = false

    close(controller_snapshotclient_for_switchos_udpsock);
    for (uint16_t tmp_global_server_logical_idx = 0; tmp_global_server_logical_idx < max_server_total_logical_num; tmp_global_server_logical_idx++) {
        close(controller_snapshotclient_for_server_udpsock_list[tmp_global_server_logical_idx]);
    }
    pthread_exit(nullptr);
}

void* run_controller_snapshotclient_cleanup_subthread(void* param) {
    snapshotclient_subthread_param_t& subthread_param = *((snapshotclient_subthread_param_t*)param);

    char sendbuf[MAX_BUFSIZE];
    snapshot_signal_t snapshot_cleanup_signal(SNAPSHOT_CLEANUP, controller_snapshotid);
    int signalsize = snapshot_cleanup_signal.serialize(sendbuf, MAX_BUFSIZE);
    // memcpy(sendbuf, &SNAPSHOT_CLEANUP, sizeof(int));
    // memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));

    char recvbuf[MAX_BUFSIZE];
    int recvsize = 0;
    bool is_timeout = false;
    while (true) {
        // char tmpip[256];
        // inet_ntop(AF_INET, &subthread_param.dstaddr.sin_addr, tmpip, 256);
        // printf("dstip %s, dstport %d, serveridx: %d\n", tmpip, ntohs(subthread_param.dstaddr.sin_port), subthread_param.global_server_logical_idx);
        udpsendto(subthread_param.udpsock, sendbuf, signalsize, 0, &subthread_param.dstaddr, subthread_param.dstaddrlen, "controller.snapshotclient.cleanup_subthread");

        // wait for SNAPSHOT_CLEANUP_ACK
        is_timeout = udprecvfrom(subthread_param.udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient.cleanup_subthread");
        if (is_timeout) {
            // printf("timeout dstip %s, dstport %d, serveridx %d", tmpip, ntohs(subthread_param.dstaddr.sin_port), subthread_param.global_server_logical_idx);
            continue;
        } else {
            snapshot_signal_t cleanupack(recvbuf, recvsize);
            INVARIANT(cleanupack.control_type() == SNAPSHOT_CLEANUP_ACK);
            // INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_CLEANUP_ACK);
            break;
        }
    }

    pthread_exit(nullptr);
}

void* run_controller_snapshotclient_start_subthread(void* param) {
    snapshotclient_subthread_param_t& subthread_param = *((snapshotclient_subthread_param_t*)param);

    char sendbuf[MAX_BUFSIZE];
    snapshot_signal_t snapshot_start_signal(SNAPSHOT_START, controller_snapshotid);
    int signalsize = snapshot_start_signal.serialize(sendbuf, MAX_BUFSIZE);
    // memcpy(sendbuf, &SNAPSHOT_START, sizeof(int));
    // memcpy(sendbuf + sizeof(int), &controller_snapshotid, sizeof(int));

    char recvbuf[MAX_BUFSIZE];
    int recvsize = 0;
    bool is_timeout = false;
    while (true) {
        udpsendto(subthread_param.udpsock, sendbuf, signalsize, 0, &subthread_param.dstaddr, subthread_param.dstaddrlen, "controller.snapshotclient.start_subthread");

        // wait for SNAPSHOT_start_ACK
        is_timeout = udprecvfrom(subthread_param.udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient.start_subthread");
        if (is_timeout) {
            continue;
        } else {
            snapshot_signal_t startack(recvbuf, recvsize);
            INVARIANT(startack.control_type() == SNAPSHOT_START_ACK);
            // INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_START_ACK);
            break;
        }
    }

    pthread_exit(nullptr);
}

void* run_controller_snapshotclient_senddata_subthread(void* param) {
    snapshotclient_subthread_param_t& subthread_param = *((snapshotclient_subthread_param_t*)param);

    char recvbuf[MAX_BUFSIZE];
    int recvsize = 0;
    bool is_timeout = false;
    while (true) {
        udpsendlarge_udpfrag(subthread_param.udpsock, controller_snapshotclient_for_server_databuf_list[subthread_param.global_server_logical_idx].array(), controller_snapshotclient_for_server_databuf_list[subthread_param.global_server_logical_idx].size(), 0, &subthread_param.dstaddr, subthread_param.dstaddrlen, "controller.snapshotclient.senddata_subthread");

        // wait for SNAPSHOT_SENDDATA_ACK
        is_timeout = udprecvfrom(subthread_param.udpsock, recvbuf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "controller.snapshotclient.senddata_subthread");
        if (is_timeout) {
            continue;
        } else {
            snapshot_signal_t senddataack(recvbuf, recvsize);
            INVARIANT(senddataack.control_type() == SNAPSHOT_SENDDATA_ACK);
            // INVARIANT(recvsize == sizeof(int) && *((int *)recvbuf) == SNAPSHOT_SENDDATA_ACK);
            break;
        }
    }

    pthread_exit(nullptr);
}

void close_controller() {
    if (controller_popserver_udpsock_list != NULL) {
        delete[] controller_popserver_udpsock_list;
        controller_popserver_udpsock_list = NULL;
    }
    if (controller_popserver_popclient_udpsock_list != NULL) {
        delete[] controller_popserver_popclient_udpsock_list;
        controller_popserver_popclient_udpsock_list = NULL;
    }
    if (controller_snapshotclient_for_server_udpsock_list != NULL) {
        delete[] controller_snapshotclient_for_server_udpsock_list;
        controller_snapshotclient_for_server_udpsock_list = NULL;
    }
    if (controller_snapshotclient_for_server_databuf_list != NULL) {
        delete[] controller_snapshotclient_for_server_databuf_list;
        controller_snapshotclient_for_server_databuf_list = NULL;
    }
    if (perserver_localcp_bandwidthcost != NULL) {
        delete[] perserver_localcp_bandwidthcost;
        perserver_localcp_bandwidthcost = NULL;
    }
}
