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
#include "concurrent_map_impl.h"
#include "message_queue_impl.h"

#include "common_impl.h"
// switchos.popworker <-> reflector.cp2dpserver
int switchos_popworker_popclient_for_reflector_udpsock = -1;
// switchos.popworker <-> controller.evictserver for cache eviction
int switchos_popworker_evictclient_for_controller_udpsock = -1;
int main() {
    parse_ini("config.ini");
    create_udpsock(switchos_popworker_evictclient_for_controller_udpsock, true, "switchos.popworker.evictclient");
    // used by popworker.evictclient
    sockaddr_in controller_evictserver_addr;
    set_sockaddr(controller_evictserver_addr, inet_addr(controller_ip_for_switchos), controller_evictserver_port);
    socklen_t controller_evictserver_addrlen = sizeof(struct sockaddr_in);

    // used by udpsocket to communicate with ptf.popserver
    sockaddr_in ptf_popserver_addr;
    set_sockaddr(ptf_popserver_addr, inet_addr("127.0.0.1"), switchos_ptf_popserver_port);
    int ptf_popserver_addr_len = sizeof(struct sockaddr);

    char pktbuf[MAX_BUFSIZE];
    uint32_t pktsize = 0;
    // recv CACHE_POP_INSWITCH_ACK and CACHE_EVICT_ACK from controlelr
    char ackbuf[MAX_BUFSIZE];
    int ack_recvsize = 0;
    // (2) communicate with ptf.popserver
    char ptfbuf[MAX_BUFSIZE];
    uint32_t ptf_sendsize = 0;
    int ptf_recvsize = 0;
    uint16_t switchos_freeidx = 0;
    // load evictdata from ptf
    uint16_t switchos_evictidx = 0;
    val_t switchos_evictvalue = val_t();
    uint32_t switchos_evictseq = 0;
    bool switchos_evictstat = false;

    int res = 0;

    // workload parser
    ParserIterator* iter = NULL;
#ifdef USE_YCSB
    iter = new YcsbParserIterator(raw_warmup_workload_filename);
#elif defined USE_SYNTHETIC
    iter = new SyntheticParserIterator(raw_warmup_workload_filename);
#endif
    INVARIANT(iter != NULL);

    netreach_key_t tmpkey = iter->key();

    ptf_sendsize = serialize_remove_cache_lookup(ptfbuf, tmpkey);
    udpsendto(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, ptf_sendsize, 0, &ptf_popserver_addr, ptf_popserver_addr_len, "switchos.popworker.popclient_for_ptf");
    udprecvfrom(switchos_popworker_popclient_for_ptf_udpsock, ptfbuf, MAX_BUFSIZE, 0, NULL, NULL, ptf_recvsize, "switchos.popworker.popclient_for_ptf");
    INVARIANT(*((int*)ptfbuf) == SWITCHOS_REMOVE_CACHE_LOOKUP_ACK);  // wait for SWITCHOS_REMOVE_CACHE_LOOKUP_ACK
    netcache_cache_evict_t tmp_netcache_cache_evict(CURMETHOD_ID, tmpkey, tmpkey.get_hashpartition_idx(switch_partition_count, max_server_total_logical_num));
    pktsize = tmp_netcache_cache_evict.serialize(pktbuf, MAX_BUFSIZE);
    while (true) {
        // printf("send NETCACHE_CACHE_EVICT to controller.evictserver\n");
        // dump_buf(pktbuf, pktsize);
        udpsendto(switchos_popworker_evictclient_for_controller_udpsock, pktbuf, pktsize, 0, &controller_evictserver_addr, controller_evictserver_addrlen, "switchos.popworker.evictclient_for_controller");

        // wait for NETCACHE_CACHE_EVICT_ACK from controller.evictserver
        // NOTE: no concurrent CACHE_EVICTs -> use request-and-reply manner to wait for entire eviction workflow
        bool is_timeout = udprecvfrom(switchos_popworker_evictclient_for_controller_udpsock, ackbuf, MAX_BUFSIZE, 0, NULL, NULL, ack_recvsize, "switchos.popworker.evictclient_for_controller");
        if (unlikely(is_timeout)) {
            continue;
        } else {
            netcache_cache_evict_ack_t tmp_cache_evict_ack(CURMETHOD_ID, ackbuf, ack_recvsize);
            INVARIANT(tmp_cache_evict_ack.key() == cur_evictkey);
            break;
        }
    }
}