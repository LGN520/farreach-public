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

int main(){
    printf("%x %x %x\n", (htonl(0x9f10)>> 16),uint16_t(htonl(uint16_t(packet_type_t::BACKUPACK))),uint16_t(packet_type_t::BACKUPACK));
}