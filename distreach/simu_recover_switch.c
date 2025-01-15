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

#include "../common/dynamic_array.h"
#include "../common/helper.h"
#include "../common/iniparser/iniparser_wrapper.h"
#include "../common/io_helper.h"
#include "../common/key.h"
#include "../common/packet_format_impl.h"
#include "../common/socket_helper.h"
#include "../common/special_case.h"
#include "../common/val.h"
#include "common_impl.h"
#include "concurrent_map_impl.h"
#include "message_queue_impl.h"

template <class key_t, class val_t>
class RevoverPkttofino : public Packet<key_t> {  // op_hdr +  vallen&value + shadowtype + seq + inswitch_hdr + frequenccy +  validvalue
public:
    RevoverPkttofino() {}
    RevoverPkttofino(method_t methodid, key_t key, val_t val);
    RevoverPkttofino(method_t methodid, const char* data, uint32_t recv_size);

    val_t val() const {
        return _val;
    }

    virtual uint32_t serialize(char* const data, uint32_t max_size);
    uint32_t seq() { return _seq; }
    uint32_t largevalueseq() { return _largevalueseq; }
    uint32_t cache_frequency() { return _cache_frequency; }
    uint16_t reg_meta_clientsid() { return _reg_meta_clientsid; }
    uint8_t validvalue() { return _validvalue; }

    int is_cached() { return (_reg_meta_clientsid & 0x4000) >> 14; }
    int is_deleted() { return (_reg_meta_clientsid & 0x0004) >> 2; }
    int is_latest() { return (_reg_meta_clientsid & 0x0002) >> 1; }
    int is_found() { return (_reg_meta_clientsid) & 1; }

    uint16_t freeidx() { return _idx; }

protected:
    virtual uint32_t size();
    virtual void deserialize(const char* data, uint32_t recv_size);
    val_t _val;
    uint32_t _seq = 0;
    uint32_t _largevalueseq = 0;
    uint32_t _cache_frequency = 0;
    uint16_t _reg_meta_clientsid = 0;
    uint16_t _idx = 0;
    uint8_t _validvalue = 0;
};

// RevoverPkttofino

template <class key_t, class val_t>
RevoverPkttofino<key_t, val_t>::RevoverPkttofino(method_t methodid, key_t key, val_t val)
    : Packet<key_t>(methodid, packet_type_t::BACKUP, key), _val(val) {
    // this->_type = optype_t(packet_type_t::BACKUP);
    INVARIANT(this->_val.val_length <= val_t::SWITCH_MAX_VALLEN);
}

template <class key_t, class val_t>
RevoverPkttofino<key_t, val_t>::RevoverPkttofino(method_t methodid, const char* data, uint32_t recvsize) {
    this->_methodid = methodid;
    this->deserialize(data, recvsize);
}

template <class key_t, class val_t>
uint32_t RevoverPkttofino<key_t, val_t>::serialize(char* const data, uint32_t max_size) {
    // op_hdr +  vallen&value + shadowtype + seq + inswitch_hdr + frequenccy +  validvalue
    char* begin = data;
    uint32_t tmp_ophdrsize = this->serialize_ophdr(begin, max_size);
    begin += tmp_ophdrsize;
    uint32_t tmp_valsize = this->_val.serialize(begin, max_size - uint32_t(begin - data));
    begin += tmp_valsize;
    uint32_t tmp_shadowtypesize = serialize_packet_type(this->_type, begin, max_size - uint32_t(begin - data));  // shadowtype
    begin += tmp_shadowtypesize;
    uint32_t bigendian_seq = htonl(this->_seq);
    memcpy(begin, (void*)&bigendian_seq, sizeof(uint32_t));  // little-endian to big-endian
    begin += sizeof(uint32_t);
    if (this->_methodid == FARREACH_ID) {  // seq_hdr.largevalueseq
        // printf("[debug]\n");
        bigendian_seq = htonl(0);
        memcpy(begin, (void*)&bigendian_seq, sizeof(uint32_t));  // little-endian to big-endian
        begin += sizeof(uint32_t);
    }
    // inswitch_hdr
    int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
    memset(begin, 0, tmp_inswitch_prev_bytes);  // the first bytes of inswitch_hdr
    begin += tmp_inswitch_prev_bytes;

    uint16_t bigendian_freeidx = htons(uint16_t(this->_idx));
    memcpy(begin, (void*)&bigendian_freeidx, sizeof(uint16_t));  // little-endian to big-endian
    begin += sizeof(uint16_t);

    // frequenccy
    uint32_t bigendian_frequenccy = htons(uint32_t(this->_cache_frequency));
    memcpy(begin, (void*)&bigendian_frequenccy, sizeof(uint32_t));  // little-endian to big-endian
    begin += sizeof(uint32_t);

    // validvalue
    uint8_t bigendian_validvalue = htons(uint8_t(this->_validvalue));
    memcpy(begin, (void*)&bigendian_validvalue, sizeof(uint8_t));  // little-endian to big-endian
    begin += sizeof(uint8_t);
    // // memcpy(begin, (void*)&this->_stat, sizeof(bool));
    // begin += sizeof(bool);               // stat_hdr.stat
    // memset(begin, 0, sizeof(uint16_t));  // stat_hdr.nodeidx_foreval
    // begin += sizeof(uint16_t);
    // begin += Packet<key_t>::get_stat_padding_bytes(this->_methodid);
    // // mark 0 for backup_hdr
    // memset(begin, 0, sizeof(uint32_t));
    // begin += sizeof(uint32_t);
    // memset(begin, 0, sizeof(uint32_t));
    // begin += sizeof(uint32_t);
    // memset(begin, 0, sizeof(uint32_t));
    // begin += sizeof(uint32_t);
    return uint32_t(begin - data);
}
template <class key_t, class val_t>
void RevoverPkttofino<key_t, val_t>::deserialize(const char* data, uint32_t recv_size) {
    // op_hdr +  vallen&value + shadowtype + seq + inswitch_hdr + frequenccy +  validvalue
    const char* begin = data;
    uint32_t tmp_ophdrsize = this->deserialize_ophdr(begin, recv_size);
    begin += tmp_ophdrsize;
    uint32_t tmp_valsize = this->_val.deserialize(begin, recv_size - uint32_t(begin - data));
    begin += tmp_valsize;
    // skip shadowtype
    begin += sizeof(uint16_t);
    // seq
    memcpy((void*)&this->_seq, begin, sizeof(uint32_t));
    this->_seq = ntohl(this->_seq);
    begin += sizeof(uint32_t);
    memcpy((void*)&this->_largevalueseq, begin, sizeof(uint32_t));
    this->_largevalueseq = ntohl(this->_largevalueseq);
    begin += sizeof(uint32_t);
    // inswitch_hdr
    memcpy((void*)&this->_reg_meta_clientsid, begin, sizeof(uint16_t));
    this->_reg_meta_clientsid = ntohs(this->_reg_meta_clientsid);
    begin += sizeof(uint16_t);

    int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
    begin += (tmp_inswitch_prev_bytes - sizeof(uint16_t));

    memcpy((void*)&this->_idx, begin, sizeof(uint16_t));

    this->_idx = ntohs(this->_idx);
    // printf("%d\n",this->_idx);
    begin += sizeof(uint16_t);
    //  frequenccy +  validvalue
    memcpy((void*)&this->_cache_frequency, begin, sizeof(uint32_t));
    this->_cache_frequency = ntohl(this->_cache_frequency);
    begin += sizeof(uint32_t);
    memcpy((void*)&this->_validvalue, begin, sizeof(uint8_t));
    this->_largevalueseq = ntohl(this->_validvalue);
    // begin += sizeof(uint8_t);
    // // skip inswitch hdr 5*32
    // int tmp_inswitch_prev_bytes = Packet<key_t>::get_inswitch_prev_bytes(this->_methodid);
    // begin += tmp_inswitch_prev_bytes;
    // memcpy((void*)&this->_freeidx, begin, sizeof(uint16_t));
    // this->_seq = ntohl(this->_freeidx);  // Big-endian to little-endian
    // begin += sizeof(uint16_t);
    // // skip stat
    // begin += sizeof(uint32_t);
    // deserialize backup
    // cache_frequency 32
    // memcpy((void*)&this->cache_frequency, begin, sizeof(uint32_t));
    // this->_seq = ntohl(this->cache_frequency);  // Big-endian to little-endian
    // begin += sizeof(uint32_t);

    // // largevalueseq 32
    // memcpy((void*)&this->largevalueseq, begin, sizeof(uint32_t));
    // this->_seq = ntohl(this->largevalueseq);  // Big-endian to little-endian
    // begin += sizeof(uint32_t);
    // // vallen 16
    // memcpy((void*)&this->vallen, begin, sizeof(uint16_t));
    // this->_seq = ntohl(this->vallen);  // Big-endian to little-endian
    // begin += sizeof(uint16_t);
    // // reg_meta 16
    // memcpy((void*)&this->reg_meta, begin, sizeof(uint16_t));
    // this->_seq = ntohl(this->reg_meta);  // Big-endian to little-endian
    begin += sizeof(uint16_t);
}
template <class key_t, class val_t>
uint32_t RevoverPkttofino<key_t, val_t>::size() {  // unused
    return 0;
    // return sizeof(optype_t) + sizeof(key_t) + sizeof(uint16_t) + val_t::MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + INSWITCH_PREV_BYTES + sizeof(uint16_t) + DEBUG_BYTES + backup_hdr;
    // return Packet<key_t>::get_ophdrsize(this->_methodid) + sizeof(uint16_t) + val_t::SWITCH_MAX_VALLEN + sizeof(optype_t) + sizeof(uint32_t) + Packet<key_t>::get_inswitch_prev_bytes(this->_methodid) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint16_t) + Packet<key_t>::get_stat_padding_bytes(this->_methodid) + sizeof(uint32_t) * 3;
}

typedef RevoverPkttofino<netreach_key_t, val_t> recoverpkt_t_t;
// 流程
// [x] send recover start
// switchos send key & idx 使用SWITCHOS_ADD_CACHE_LOOKUP
//      note: 可能有些idx下没有key，用control type 标志empty key or cached key
// [x] switchos send finish
// [x] recv recover_end
// [x] wait for acknum >= max kv bkt num

// recover worker
// [x]读取队列
// 发送给switch RevoverPkttofino
// 批量发

// recover server
// [x]收到来自switchos的 key&idx 写入到worker队列中去，向switchos发回 ack
// 收到来自switch的ack，说明有一个key的value已经写好了，直接让popserver写 cachelookup table
// 收到switch的ack，通过backup_hdr的标记判断是不是最新的，决定是否要发送给server做持久化

// n-th item is recovered
bool idx_cached_is_recovered[MQ_SIZE];
// n-th item is persist
bool idx_cached_is_persist[MQ_SIZE];
// n-th item key
// Key idx_cached_key[MQ_SIZE];
struct recover_key_idx* idx_cached_key[MQ_SIZE];
int recover_ack_num = 0;
int persistent_ack_num = 0;
uint16_t idx_cached_meta[MQ_SIZE];
Val idx_cached_val[MQ_SIZE];
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
inline uint32_t serialize_remove_cache_lookup(char* buf, netreach_key_t key);
inline uint32_t serialize_add_cache_lookup(char* buf, netreach_key_t key, uint16_t freeidx);
void prepare_recover();
int local_server_idx = -1;
bool volatile switchos_running = false;
bool volatile main_recover_finish = false;
bool volatile key_recover_finish = false;
// 一个server 收key
// 一个worker 发recover包
int main(int argc, char* argv[]) {
    // parse the first para ,idx
    // if(argc < 2){
    //     printf("Usage: %s <local_server_idx>\n", argv[0]);
    //     exit(1);
    // }
    // local_server_idx = atoi(argv[1]);
    
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
    while (!key_recover_finish) {
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
#ifdef PIPILINE_RECOVER
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
    // printf("[debug]send FETCH_RECOVERKEY_START\n");
    fflush(stdout);
    // recv recover_end
    // fecth all key
    while (true) {
        bool is_timeout = false;
        is_timeout = udprecvfrom(recover_start_end_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "switchos.recover.sync");
        if (unlikely(is_timeout)) {
            continue;
        }
        // printf("[debug] recv FETCH_RECOVERKEY_END to\n");
        INVARIANT(((int*)buf)[0] == FETCH_RECOVERKEY_END);
        break;
    }
    // waiting for recover switch
    while (recover_ack_num < switch_kv_bucket_num) {
    }

    // printf("[debug] recover_ack_num > switch_kv_bucket_num finished\n");
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
#endif
void* main_recover(void* param) {
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

    // send recover_start
    bool is_timeout = false;
    ((int*)buf)[0] = FETCH_RECOVERKEY_START;
    // printf("[debug]\n");fflush(stdout);
    udpsendto(recover_start_end_udpsock, buf, sizeof(int), 0, &switchos_sync_addr, switchos_sync_addrlen, "switchos.recover.sync");
    // printf("[debug]send FETCH_RECOVERKEY_START\n");
    fflush(stdout);
    // recv recover_end
    // fecth all key
    // while (true) {
    //     bool is_timeout = false;
    //     is_timeout = udprecvfrom(recover_start_end_udpsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recvsize, "switchos.recover.sync");
    //     if (unlikely(is_timeout)) {
    //         continue;
    //     }
    //     // printf("[debug] recv FETCH_RECOVERKEY_END to\n");
    //     INVARIANT(((int*)buf)[0] == FETCH_RECOVERKEY_END);
    //     break;
    // }
    // waiting for recover switch
    while (recover_ack_num < switch_kv_bucket_num) {
    }

    // printf("[debug] recover_ack_num > switch_kv_bucket_num finished\n");
    // CUR_TIME(recover_t2);
    // DELTA_TIME(recover_t2, recover_t1, recover_t3);
    // printf("[Statistics] recover switch: %f s w/ cache size %d\n", GET_MICROSECOND(recover_t3) / 1000.0 / 1000.0, switch_kv_bucket_num);
    // fflush(stdout);
    while (persistent_ack_num < switch_kv_bucket_num) {
    }
    // CUR_TIME(recover_t2);
    // DELTA_TIME(recover_t2, recover_t1, recover_t3);
    // printf("[Statistics] persist server: %f s w/ cache size %d\n", GET_MICROSECOND(recover_t3) / 1000.0 / 1000.0, switch_kv_bucket_num);
    // fflush(stdout);
    main_recover_finish = true;
    while (!key_recover_finish) {
    }
    close(recover_start_end_udpsock);
    pthread_exit(nullptr);
}

void prepare_recover() {
    // prepare recover
    prepare_udpserver(recover_server_for_switchos_udpsock, true, recover_server_port, "switchos.recoversync", 0, 1000);
    create_udpsock(recover_start_end_udpsock, true, "recover.start_end", 0, SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS);
    create_udpsock(recover_worker_for_switch_udpsock, false, "recover.worker");  //, 0, SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS);
    create_udpsock(recover_popworker_popclient_for_ptf_udpsock, false, "recover.popworker.popclient_for_ptf");
    create_udpsock(fake_client_udpsock, true, "recover.fake.client", 0, SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS);

    memory_fence();

    printf("[recover] prepare end\n");
}

void* run_recover_server(void* param) {
    recover_thread_count++;
}
void* run_recover_worker(void* param) {
    struct timespec recover_t1, recover_t2, recover_t3;
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

    struct sockaddr_in client_addr;
    set_sockaddr(client_addr, inet_addr(client_ips[0]), 5008);  // port 没用反正会回来
    socklen_t client_addrlen = sizeof(struct sockaddr_in);
    int tmpudpsock_for_reflector = -1;
    create_udpsock(tmpudpsock_for_reflector, true, "switchos.recover.udpsock_for_reflector", 0, SWITCHOS_POPCLIENT_FOR_REFLECTOR_TIMEOUT_USECS);  // to reduce snapshot latency
    sockaddr_in reflector_cp2dpserver_addr;
    set_sockaddr(reflector_cp2dpserver_addr, inet_addr(reflector_ip_for_switchos), reflector_cp2dpserver_port);
    int reflector_cp2dpserver_addr_len = sizeof(struct sockaddr);
    char ptfbuf[MAX_BUFSIZE];
    uint32_t ptf_sendsize = 0;
    int ptf_recvsize = 0;
    char buf[MAX_BUFSIZE];
    int recvsize = 0;
    int req_size = 0;
    int controltype = -1;

    char pktbuf[MAX_BUFSIZE];
    uint32_t pktsize = 0;
    // int sender_counter = 0;
    char ackbuf[MAX_BUFSIZE];
    int ack_recvsize = 0;
    int worker_send_counter = 0;
    struct recover_key_idx* tmp_recover_key_idx;

    printf("[recover.worker] ready\n");
    printf("[recover.server] ready\n");
    recover_thread_count++;
    while (!switchos_running) {
    }
    // 1.fetch key from switchos
    // 2.send switch recover pkt
    // 3.do persistence in server
    while (switchos_running) {
        // 1.fetch key from switchos
        printf("[debug] fetch key from prepared_files\n");
        CUR_TIME(recover_t1);
        std::ifstream infile("prepared_key.out", std::ios::binary);
        for (int i = 0; i < switch_kv_bucket_num; ++i) {
            if (idx_cached_key[i] == nullptr) {
                idx_cached_key[i] = new recover_key_idx();
            }
            infile.read(reinterpret_cast<char*>(&idx_cached_key[i]->idx), sizeof(uint16_t));
            infile.read(reinterpret_cast<char*>(&idx_cached_key[i]->recoverkey.keylolo), sizeof(uint32_t));
            infile.read(reinterpret_cast<char*>(&idx_cached_key[i]->recoverkey.keylohi), sizeof(uint32_t));
            infile.read(reinterpret_cast<char*>(&idx_cached_key[i]->recoverkey.keyhilo), sizeof(uint32_t));
            infile.read(reinterpret_cast<char*>(&idx_cached_key[i]->recoverkey.keyhihi), sizeof(uint32_t));
            idx_cached_is_recovered[idx_cached_key[i]->idx] = false;
            idx_cached_is_persist[idx_cached_key[i]->idx] = false;
            worker_send_counter++;
        }
        infile.close();


        CUR_TIME(recover_t2);
        DELTA_TIME(recover_t2, recover_t1, recover_t3);
        printf("[Statistics] fetch key: %f s w/ cache size %d\n", GET_MICROSECOND(recover_t3) / 1000.0 / 1000.0, switch_kv_bucket_num);
        fflush(stdout);



        CUR_TIME(recover_t1);
        for (int i = 0; i < switch_kv_bucket_num; i++) {
            recoverpkt_t tmp_recoverpkt(CURMETHOD_ID, idx_cached_key[i]->recoverkey, idx_cached_val[i], 0, idx_cached_key[i]->idx, true, false);
            pktsize = tmp_recoverpkt.serialize(pktbuf, MAX_BUFSIZE);
            udpsendto(recover_worker_for_switch_udpsock, pktbuf, pktsize, 0, &client_addr, client_addrlen, "recover.send");

            ptf_sendsize = serialize_add_cache_lookup(ptfbuf, idx_cached_key[i]->recoverkey, i);
            udpsendto(recover_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
            // udprecvfrom(recover_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
            // INVARIANT(*((int*)ptfbuf) == SWITCHOS_ADD_CACHE_LOOKUP_ACK);  // wait for SWITCHOS_ADD_CACHE_LOOKUP_ACK
            setvalid_inswitch_t tmp_setvalid_req(CURMETHOD_ID, idx_cached_key[i]->recoverkey, i, 1);
            pktsize = tmp_setvalid_req.serialize(pktbuf, MAX_BUFSIZE);
            udpsendto(tmpudpsock_for_reflector, pktbuf, pktsize, 0, &reflector_cp2dpserver_addr, reflector_cp2dpserver_addr_len, "switchos.recover.udpsock_for_reflector");
        }
        CUR_TIME(recover_t2);
        DELTA_TIME(recover_t2, recover_t1, recover_t3);
        printf("[Statistics] recover switch: %f s w/ cache size %d\n", GET_MICROSECOND(recover_t3) / 1000.0 / 1000.0, switch_kv_bucket_num);

        printf("[debug] do persistence in server\n");
        // 3.do persistence in server
        recover_ack_num =  switch_kv_bucket_num;
        persistent_ack_num = switch_kv_bucket_num;
        key_recover_finish = true;
        break;
    }

    printf("[recover server] exit\n");
    // switchos_popserver_finish = true;
    close(fake_client_udpsock);
    close(recover_popworker_popclient_for_ptf_udpsock);
    close(recover_server_for_switchos_udpsock);
    close(tmpudpsock_for_reflector);
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
inline uint32_t serialize_remove_cache_lookup(char* buf, netreach_key_t key) {
    memcpy(buf, &SWITCHOS_REMOVE_CACHE_LOOKUP, sizeof(int));
    uint32_t tmp_keysize = key.serialize(buf + sizeof(int), MAX_BUFSIZE - sizeof(int));
    return sizeof(int) + tmp_keysize;
}
