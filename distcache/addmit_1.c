#include <arpa/inet.h>  // inetaddr conversion
#include <fcntl.h>
#include <getopt.h>
#include <netinet/in.h>  // struct sockaddr_in
#include <stdlib.h>
#include <sys/socket.h>  // socket API
#include <sys/types.h>
#include <unistd.h>
#include <atomic>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <vector>
// #include <sys/time.h> // struct timeval

#include "../common/helper.h"
#include "../common/iniparser/iniparser_wrapper.h"
#include "../common/key.h"
#include "../common/latency_helper.h"
#include "../common/socket_helper.h"
#include "../common/val.h"

#ifdef USE_YCSB
#include "../common/workloadparser/ycsb_parser.h"
#elif defined USE_SYNTHETIC
#include "../common/workloadparser/synthetic_parser.h"
#endif

#include "common_impl.h"

int main(int argc, char** argv) {
    parse_ini("config.ini");
    int res = 0;

    // workload parser
    ParserIterator* iter = NULL;
#ifdef USE_YCSB
    iter = new YcsbParserIterator(raw_warmup_workload_filename);
#elif defined USE_SYNTHETIC
    iter = new SyntheticParserIterator(raw_warmup_workload_filename);
#endif
    INVARIANT(iter != NULL);

    netreach_key_t tmpkey;
    val_t tmpval;

    // for network communication
    char buf[MAX_BUFSIZE];
    int req_size = 0;
    int recv_size = 0;
    int clientsock = -1;
    create_udpsock(clientsock, false, "warmup_client");
    struct sockaddr_in server_addr;
    set_sockaddr(server_addr, inet_addr(server_ips[0]), server_worker_port_start);
    socklen_t server_addrlen = sizeof(struct sockaddr_in);
    printf("[warmup_client] cache size: %d\n", switch_kv_bucket_num);
    fflush(stdout);
    int tmp_warmupcnt = 0;

    // printf("tmp_warmupcnt %d\n",tmp_warmupcnt++);fflush(stdout);
    tmpkey = iter->key();
    if (iter->type() == uint8_t(packet_type_t::WARMUPREQ)) {  // update or insert
        // tmpval = iter->val();
        // INVARIANT(tmpval.val_length <= val_t::SWITCH_MAX_VALLEN);
        // warmup_request_t req(tmpkey, tmpval);

        warmup_request_t req(CURMETHOD_ID, tmpkey);

        req_size = req.serialize(buf, MAX_BUFSIZE);
#ifdef DUMP_BUF
        dump_buf(buf, req_size);
#endif

        udpsendto(clientsock, buf, req_size, 0, &server_addr, server_addrlen, "ycsb_remove_client");

        udprecvfrom(clientsock, buf, MAX_BUFSIZE, 0, NULL, NULL, recv_size, "ycsb_remote_client");
        INVARIANT(recv_size > 0);

        warmup_ack_t rsp(CURMETHOD_ID, buf, recv_size);
        UNUSED(rsp);

        usleep(50);  // avoid switch misbehavior under a burst of server-issued valueupdate due to Tofino bug?
    } else {
        printf("Invalid request type: %u\n", uint32_t(iter->type()));
        exit(-1);
    }

    iter->closeiter();
    delete iter;
    iter = NULL;

    close(clientsock);

    return;
}