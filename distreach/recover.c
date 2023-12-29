#include <arpa/inet.h>  // inetaddr conversion
#include <getopt.h>
#include <signal.h>  // for signal and raise
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include "../common/helper.h"
#include "../common/io_helper.h"
#include "../common/key.h"
#include "../common/val.h"

#include "../common/dynamic_array.h"
#include "../common/iniparser/iniparser_wrapper.h"
#include "../common/packet_format_impl.h"
#include "../common/socket_helper.h"
#include "../common/special_case.h"
#include "concurrent_map_impl.h"
#include "message_queue_impl.h"

#include "common_impl.h"

// 流程
// [x] send recover start
// switchos send key & idx 使用SWITCHOS_ADD_CACHE_LOOKUP
//      note: 可能有些idx下没有key，用control type 标志empty key or cached key
// [x] switchos send finish
// [x] recv recover_end
// [x] wait for acknum >= max kv bkt num

// recover worker
// [x]读取队列
// 发送给switch RecoverPkt
// 批量发

// recover server
// [x]收到来自switchos的 key&idx 写入到worker队列中去，向switchos发回 ack
// 收到来自switch的ack，说明有一个key的value已经写好了，直接让popserver写 cachelookup table
// 收到switch的ack，通过backup_hdr的标记判断是不是最新的，决定是否要发送给server做持久化

// n-th item is recovered
bool idx_cached_is_recovered[MQ_SIZE];
// n-th item key
Key idx_cached_key[MQ_SIZE];
int recover_ack_num = 0;
int persistent_ack_num = 0;
struct recover_key_idx {
    Key recoverkey;
    uint16_t idx;
};
MessagePtrQueue<recover_key_idx> recover_key_idx_ptr_queue(MQ_SIZE);

// recover.server <-> switchos send key and freeidx
int recover_server_for_switchos_udpsock = -1;
// recover.worker <-> switch sync
int recover_worker_for_switch_udpsock = -1;
// recover.server controller start and end;
int recover_start_end_udpsock = -1;
// recover <-> ptf_popserver
int recover_popworker_popclient_for_ptf_udpsock = -1;
// recover <-> server
int fake_client_udpsock = -1;

int all_recover_thread_count = 3;
int recover_thread_count = 0;
void* run_recover_server(void* param);
void* run_recover_worker(void* param);
void* main_recover(void* param);

inline uint32_t serialize_add_cache_lookup(char* buf, netreach_key_t key, uint16_t freeidx);
void prepare_recover();

bool volatile switchos_running = false;
bool volatile main_recover_finish = false;
// 一个server 收key
// 一个worker 发recover包
int main() {
    parse_ini("config.ini");
    parse_control_ini("control_type.ini");
    prepare_recover();

    pthread_t recoverserver_thread, recoverworker_thread, main_recover_thread;
    int ret;
    ret = pthread_create(&recoverserver_thread, nullptr, run_recover_server, nullptr);
    if (ret) {
        COUT_N_EXIT("Error: " << ret);
    }
    ret = pthread_create(&recoverworker_thread, nullptr, run_recover_worker, nullptr);
    if (ret) {
        COUT_N_EXIT("Error: " << ret);
    }
    ret = pthread_create(&main_recover_thread, nullptr, main_recover, nullptr);
    if (ret) {
        COUT_N_EXIT("Error: " << ret);
    }
    while (recover_thread_count < all_recover_thread_count)
        sleep(1);
    printf("[recover] all threads ready\n");
    switchos_running = true;
    // recover();
    while (!main_recover_finish) {
    }
    switchos_running = false;
    printf("[recover] stop\n");
    void* status;
    int rc = pthread_join(recoverworker_thread, &status);
    if (rc) {
        COUT_N_EXIT("Error:unable to join," << rc);
    }
    rc = pthread_join(recoverserver_thread, &status);
    if (rc) {
        COUT_N_EXIT("Error:unable to join," << rc);
    }
    rc = pthread_join(main_recover_thread, &status);
    if (rc) {
        COUT_N_EXIT("Error:unable to join," << rc);
    }
    free_common();
    printf("[recover] all threads end\n");
}
void* main_recover(void* param) {
    struct timespec recover_t1, recover_t2, recover_t3;
    struct sockaddr_in switchos_sync_addr;
    set_sockaddr(switchos_sync_addr, inet_addr(switchos_ip), switchos_recover_sync_port);
    socklen_t switchos_sync_addrlen = sizeof(struct sockaddr_in);
    char buf[MAX_BUFSIZE];
    int recvsize = 0;
    recover_thread_count++;
    while (!switchos_running) {
    }
    printf("[main_recover] start\n");
    fflush(stdout);
    CUR_TIME(recover_t1);
    // send recover_start
    bool is_timeout = false;
    ((int*)buf)[0] = FETCH_RECOVERKEY_START;
    // printf("[debug]\n");fflush(stdout);
    udpsendto(recover_start_end_udpsock, buf, sizeof(int), 0, &switchos_sync_addr, switchos_sync_addrlen, "switchos.recover.sync");
  //printf("[debug]send FETCH_RECOVERKEY_START\n");
    fflush(stdout);
    // recv recover_end
    // fecth all key
    while (true) {
        bool is_timeout = false;
        is_timeout = udprecvfrom(recover_start_end_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "switchos.recover.sync");
        if (unlikely(is_timeout)) {
            continue;
        }
      //printf("[debug] recv FETCH_RECOVERKEY_END to\n");
        INVARIANT(((int*)buf)[0] == FETCH_RECOVERKEY_END);
        break;
    }
    // waiting for recover switch
    while (recover_ack_num < switch_kv_bucket_num) {
    }

  //printf("[debug] recover_ack_num > switch_kv_bucket_num finished\n");
    CUR_TIME(recover_t2);
    DELTA_TIME(recover_t2, recover_t1, recover_t3);
    printf("[Statistics] recover switch: %f s w/ cache size %d\n", GET_MICROSECOND(recover_t3) / 1000.0 / 1000.0, switch_kv_bucket_num);
    fflush(stdout);
    while (persistent_ack_num < switch_kv_bucket_num) {
    }
    CUR_TIME(recover_t2);
    DELTA_TIME(recover_t2, recover_t1, recover_t3);
    printf("[Statistics] persist server: %f s w/ cache size %d\n", GET_MICROSECOND(recover_t3) / 1000.0 / 1000.0, switch_kv_bucket_num);
    fflush(stdout);
    main_recover_finish = true;
    close(recover_start_end_udpsock);
    pthread_exit(nullptr);
}

void prepare_recover() {
    // prepare recover
    prepare_udpserver(recover_server_for_switchos_udpsock, false, recover_server_port, "switchos.recoversync");
    create_udpsock(recover_start_end_udpsock, true, "recover.start_end", 0, 5 * SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS);
    create_udpsock(recover_worker_for_switch_udpsock, false, "recover.worker");  //, 0, 5 * SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS);
    create_udpsock(recover_popworker_popclient_for_ptf_udpsock, false, "recover.popworker.popclient_for_ptf");
    create_udpsock(fake_client_udpsock, false, "recover.fake.client");

    memory_fence();

    printf("[recover] prepare end\n");
}
void* run_recover_server(void* param) {
    // NOTE: controller.popclient address continues to change
    struct sockaddr_in switchos_recover_client_addr;
    socklen_t switchos_recover_client_addrlen = sizeof(struct sockaddr_in);

    struct sockaddr_in ptf_popserver_addr;
    set_sockaddr(ptf_popserver_addr, inet_addr(switchos_ip), switchos_ptf_popserver_port);
    int ptf_popserver_addr_len = sizeof(struct sockaddr);
    
    std::vector<struct sockaddr_in> server_addrs;
    socklen_t server_addrlen = sizeof(struct sockaddr_in);
    for (auto server_ip_for_controller : server_ip_for_controller_list) {
        struct sockaddr_in tmp_sockaddr;
        set_sockaddr(tmp_sockaddr, inet_addr(server_ip_for_controller), server_worker_port_start);
        server_addrs.push_back(tmp_sockaddr);
    }
    char ptfbuf[MAX_BUFSIZE];
    uint32_t ptf_sendsize = 0;
    int ptf_recvsize = 0;
    char buf[MAX_BUFSIZE];
    int recvsize = 0;
    int req_size = 0;
    int controltype = -1;

    printf("[recover.server] ready\n");
    recover_thread_count++;
    while (!switchos_running) {
    }
    // printf("[recover.server] running\n");fflush(stdout);
    // Process CACHE_POP packet <optype, key, vallen, value, seq, serveridx>

    while (switchos_running) {
        //  printf("[recover.server] running\n");fflush(stdout);
        udprecvfrom(recover_server_for_switchos_udpsock, buf, MAX_BUFSIZE, 0, &switchos_recover_client_addr, &switchos_recover_client_addrlen, recvsize, "switchos.popserver");

        controltype = ((int*)buf)[0];
      //printf("[debug][recover_server]recv a packte %d %d\n", controltype, htonl(controltype));
        fflush(stdout);
        if (controltype == EMPTY_CACHE_LOOKUP) {
            // empty entry
            idx_cached_is_recovered[((int*)buf)[1]] = true;
          //printf("[debug][recover_server] recv empty entry %d\n", ((int*)buf)[1]);
            recover_ack_num++;
            persistent_ack_num++;
            ((int*)buf)[0] = SWITCHOS_ADD_CACHE_LOOKUP_ACK;
            udpsendto(recover_server_for_switchos_udpsock, buf, sizeof(int), 0, &switchos_recover_client_addr, switchos_recover_client_addrlen, "switchos.recover.popack");

            fflush(stdout);
        } else if (controltype == SWITCHOS_ADD_CACHE_LOOKUP) {
            // 写进去
            struct recover_key_idx* tmp_recover_key_idx = (struct recover_key_idx*)malloc(sizeof(struct recover_key_idx));
          //printf("[debug][recover_server] gen tmp_recover_key_idx %d\n", recvsize);
            dump_buf(buf, recvsize);
            int keysize = tmp_recover_key_idx->recoverkey.deserialize(buf + sizeof(int), recvsize - sizeof(int) - sizeof(uint16_t));
            tmp_recover_key_idx->idx = *(uint16_t*)(buf + recvsize - sizeof(uint16_t));
            bool res = recover_key_idx_ptr_queue.write(tmp_recover_key_idx);  // freed by worker
            if (!res) {
                printf("[recover] message queue overflow of recover.recover_key_idx_ptr_queue!");
            }
            // 打标记
            ((int*)buf)[0] = SWITCHOS_ADD_CACHE_LOOKUP_ACK;
            udpsendto(recover_server_for_switchos_udpsock, buf, sizeof(int), 0, &switchos_recover_client_addr, switchos_recover_client_addrlen, "switchos.recover.popack");
        } else if ((htonl(((uint16_t*)buf)[0]) >> 16) == uint16_t(packet_type_t::BACKUPACK)) {
            recoverpkt_t tmp_recoverpkt(CURMETHOD_ID, buf, recvsize);
          //printf("[debug] tmp_recoverpkt %x\n", tmp_recoverpkt.freeidx());
            // switch recovered
            // send to switchos pop server
            ptf_sendsize = serialize_add_cache_lookup(ptfbuf, tmp_recoverpkt.key(), tmp_recoverpkt.freeidx());
            udpsendto(recover_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
            udprecvfrom(recover_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
            INVARIANT(*((int*)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_ACK);  // wait for SWITCHOS_ADD_CACHE_LOOKUP_ACK
            // do persistent to server
            // 4041
            // printf("[debug] tmp_recoverpkt.getreg_meta()%x\n", tmp_recoverpkt.getreg_meta());
            if (tmp_recoverpkt.getreg_meta() == 0x4140) {  // is_deleted 0 is_latest 1 validvalue 1 is_founded 1
                put_request_seq_t req(CURMETHOD_ID, tmp_recoverpkt.key(), tmp_recoverpkt.val(), 0);
                req_size = req.serialize(buf, MAX_BUFSIZE);
                int server_idx = tmp_recoverpkt.key().get_hashpartition_idx(switch_partition_count, max_server_total_logical_num);
                // dump_buf(buf,req_size);
                // printf("[debug] send persistent %d req_size%d\n", server_idx, req_size);
                udpsendto(fake_client_udpsock, buf, req_size, 0, &server_addrs[server_idx], server_addrlen, "recover.client");
                udprecvfrom(fake_client_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, req_size, "recover.client");

            } else {
                // printf("[debug] tmp_recoverpkt.getreg_meta()%x\n", tmp_recoverpkt.getreg_meta());
            }
            persistent_ack_num++;
            recover_ack_num++;
          //printf("[debug] recover_ack_num %x\n", recover_ack_num);
            if (recover_ack_num >= switch_kv_bucket_num)
                break;
        }
    }

    printf("[recover server] exit\n");
    // switchos_popserver_finish = true;
    close(fake_client_udpsock);
    close(recover_popworker_popclient_for_ptf_udpsock);
    close(recover_server_for_switchos_udpsock);
    pthread_exit(nullptr);
}
// put_request_seq_t persistent
void* run_recover_worker(void* param) {
    char pktbuf[MAX_BUFSIZE];
    uint32_t pktsize = 0;

    char ackbuf[MAX_BUFSIZE];
    int ack_recvsize = 0;
    struct recover_key_idx* tmp_recover_key_idx;
    struct sockaddr_in client_addr;
    set_sockaddr(client_addr, inet_addr(client_ips[0]), 5008);  // port 没用反正会回来
    socklen_t client_addrlen = sizeof(struct sockaddr_in);
    printf("[recover.worker] ready\n");
    recover_thread_count++;
    while (!switchos_running) {
    }
    int worker_send_counter = 0;
    while (switchos_running) {
        tmp_recover_key_idx = recover_key_idx_ptr_queue.read();
        while (worker_send_counter - recover_ack_num >= 30) {  // 防止丢包，等一等，别发太快
        }
        if (tmp_recover_key_idx == NULL) {
            continue;
        }
        val_t tmp_val;
        uint32_t tmp_seq = 0;
        recoverpkt_t tmp_recoverpkt(CURMETHOD_ID, tmp_recover_key_idx->recoverkey, tmp_val, tmp_seq, tmp_recover_key_idx->idx, 0);
        pktsize = tmp_recoverpkt.serialize(pktbuf, MAX_BUFSIZE);
        udpsendto(recover_worker_for_switch_udpsock, pktbuf, pktsize, 0, &client_addr, client_addrlen, "recover.send");
      //printf("[debug]worker_send_counter %d tmp_recover_key_idx->idx %d\n", worker_send_counter++, tmp_recover_key_idx->idx);
        free(tmp_recover_key_idx);
    }
    printf("[recover worker] exit\n");
    close(recover_worker_for_switch_udpsock);
    pthread_exit(nullptr);
}

inline uint32_t serialize_add_cache_lookup(char* buf, netreach_key_t key, uint16_t freeidx) {
    memcpy(buf, &SWITCHOS_ADD_CACHE_LOOKUP, sizeof(int));
    uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
    memcpy(buf + sizeof(int) + tmp_keysize, &freeidx, sizeof(uint16_t));
    return sizeof(int) + tmp_keysize + sizeof(uint16_t);
}