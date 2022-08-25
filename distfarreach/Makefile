-include Makefile.local

include mk/recipes.mk

DEPS :=
CLEAN :=

##############################################################################
# Compile, link, and install flags

CC := g++
#CFLAGS += -std=c++14 -O3 -g -Wall -Werror -march=native -fno-omit-frame-pointer
CFLAGS += -std=c++14 -O3 -g -Wall -march=native -fno-omit-frame-pointer
CFLAGS += $(EXTRA_CFLAGS)
CFLAGS_SHARED += $(CFLAGS) -fPIC
LDLIBS += -lpthread -ldl -lstdc++fs -lboost_system
#-ljemalloc 
LDLIBS += $(EXTRA_LDLIBS)

LDDIR += -L/usr/lib/x86_64-linux-gnu
INCDIR += -I/usr/include

##############################################################################
# DPDK configuration

# Prefix for dpdk
#RTE_SDK ?= /usr/
# mpdts to compile
DPDK_PMDS ?= ixgbe i40e tap virtio

DPDK_INCDIR += -I$(RTE_SDK)/$(RTE_TARGET)/include
DPDK_LDDIR += -L$(RTE_SDK)/$(RTE_TARGET)/lib/
DPDK_LDLIBS += \
  -Wl,--whole-archive \
   $(addprefix -lrte_pmd_,$(DPDK_PMDS)) \
  -lrte_eal \
  -lrte_mempool \
  -lrte_mempool_ring \
  -lrte_hash \
  -lrte_ring \
  -lrte_kvargs \
  -lrte_ethdev \
  -lrte_mbuf \
  -lnuma \
  -lrte_bus_pci \
  -lrte_pci \
  -lrte_cmdline \
  -lrte_timer \
  -lrte_net \
  -lrte_kni \
  -lrte_bus_vdev \
  -lrte_gso \
  -Wl,--no-whole-archive \
  -ldl \
  $(EXTRA_LIBS_DPDK)

DEPS += dpdk_helper.d
CLEAN += dpdk_helper.o
DPDK_OBJECTS += dpdk_helper.o

INCDIR += $(DPDK_INCDIR)
LDDIR += $(DPDK_LDDIR)

##############################################################################
# RocksDB configuration

ROCKSDB_INCDIR += -I../rocksdb-6.22.1/include
ROCKSDB_LDDIR += -L../rocksdb-6.22.1
# V2: Bourbon
#ROCKSDB_INCDIR += -I../rocksdb-6.22.1-model-v2/include
#ROCKSDB_LDDIR += -L../rocksdb-6.22.1-model-v2

ROCKSDB_LDLIBS = -lrocksdb -lzstd -lbz2 -llz4 -lsnappy -lz -ldl

DEPS += rocksdb_wrapper.d
CLEAN += rocksdb_wrapper.o
ROCKSDB_OBJECTS += rocksdb_wrapper.o

INCDIR += $(ROCKSDB_INCDIR)
LDDIR += $(ROCKSDB_LDDIR)

##############################################################################

# MKL configuration
MKL_INCDIR += -I/opt/intel/mkl/include
MKL_LDDIR += -L/opt/intel/mkl/lib/intel64 -Wl,-rpath=/opt/intel/mkl/lib/intel64
MKL_LDLIBS += -lmkl_rt

INCDIR += $(MKL_INCDIR)
LDDIR += $(MKL_LDDIR)

##############################################################################

# Utils

UTILS_OBJECTS :=

DEPS += key.d val.d
CLEAN += key.o val.o
UTILS_OBJECTS += key.o val.o

DEPS += socket_helper.d checksum_helper.d
CLEAN += socket_helper.o checksum_helper.o
UTILS_OBJECTS += socket_helper.o checksum_helper.o

DEPS += workloadparser/parser.d workloadparser/ycsb_parser.d workloadparser/synthetic_parser.d
CLEAN += workloadparser/parser.o workloadparser/ycsb_parser.o workloadparser/synthetic_parser.o
UTILS_OBJECTS += workloadparser/parser.o workloadparser/ycsb_parser.o workloadparser/synthetic_parser.o

DEPS += crc32.d
CLEAN += crc32.o
UTILS_OBJECTS += crc32.o

DEPS += iniparser/iniparser.d iniparser/dictionary.d iniparser/iniparser_wrapper.d
CLEAN += iniparser/iniparser.o iniparser/dictionary.o iniparser/iniparser_wrapper.o
UTILS_OBJECTS += iniparser/iniparser_wrapper.o iniparser/iniparser.o iniparser/dictionary.o

DEPS += special_case.d
CLEAN += special_case.o
UTILS_OBJECTS += special_case.o

DEPS += snapshot_record.d
CLEAN += snapshot_record.o
UTILS_OBJECTS += snapshot_record.o

DEPS += latency_helper.d
CLEAN += latency_helper.o
UTILS_OBJECTS += latency_helper.o

DEPS += io_helper.d
CLEAN += io_helper.o
UTILS_OBJECTS += io_helper.o

DEPS += dynamic_rulemap.d
CLEAN += dynamic_rulemap.o
UTILS_OBJECTS += dynamic_rulemap.o

DEPS += dynamic_array.d
CLEAN += dynamic_array.o
UTILS_OBJECTS += dynamic_array.o

DEPS += pkt_ring_buffer.d
CLEAN += pkt_ring_buffer.o
UTILS_OBJECTS += pkt_ring_buffer.o

#DEPS += seq_cache.d
#CLEAN += seq_cache.o
#UTILS_OBJECTS += seq_cache.o

##############################################################################

CPPFLAGS += $(INCDIR)
CPPFLAGS += $(EXTRA_CPPFLAGS)
LDFLAGS += -no-pie -pthread -g $(LDDIR)
LDFLAGS += $(EXTRA_LDFLAGS)

##############################################################################

TARGETS := prepare split_workload remote_client loader server switchos controller skewness_check recover/controller_get_snapshotid warmup_client preparefinish_client reflector
#recover/server_recover localtest
#dpdktest_client dpdktest_server socktest_client socktest_server

prepare: prepare.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += prepare.d
CLEAN += prepare.o prepare

localtest: localtest.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += localtest.d
CLEAN += localtest.o localtest

split_workload: split_workload.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += split_workload.d
CLEAN += split_workload.o split_workload

loader: loader.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += loader.d
CLEAN += loader.o loader

warmup_client: warmup_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += warmup_client.d
CLEAN += warmup_client.o warmup_client

remote_client: remote_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += remote_client.d
CLEAN += remote_client.o remote_client

preparefinish_client: preparefinish_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += preparefinish_client.d
CLEAN += preparefinish_client.o preparefinish_client

#server: server.o $(UTILS_OBJECTS) $(ROCKSDB_OBJECTS) $(DPDK_OBJECTS)
	#$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) $(DPDK_LDLIBS) -o $@
server: server.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += server.d
CLEAN += server.o server

switchos: switchos.o $(UTILS_OBJECTS)
	$(CC) -pthread -g $(LDDIR) $^ -lpthread -o $@
DEPS += switchos.d
CLEAN += switchos.o switchos

controller: controller.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += controller.d
CLEAN += controller.o controller

reflector: reflector.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += reflector.d
CLEAN += reflector.o reflector

#dpdktest_client: dpdktest_client.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_client.d
#CLEAN += dpdktest_client.o dpdktest_client

#dpdktest_server: dpdktest_server.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_server.d
#CLEAN += dpdktest_server.o dpdktest_server

#socktest_client: socktest_client.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_client.d
#CLEAN += socktest_client.o socktest_client

#socktest_server: socktest_server.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_server.d
#CLEAN += socktest_server.o socktest_server

skewness_check: skewness_check.o $(UTILS_OBJECTS) 
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += skewness_check.d
CLEAN += skewness_check.o skewness_check

recover/controller_get_snapshotid: recover/controller_get_snapshotid.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += recover/controller_get_snapshotid.d
CLEAN += recover/controller_get_snapshotid.o recover/controller_get_snapshotid

recover/server_recover: recover/server_recover.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += recover/server_recover.d
CLEAN += recover/server_recover.o recover/server_recover

##############################################################################
# Top level targets

all: $(TARGETS)

clean:
	rm -rf $(CLEAN) $(DEPS)

.DEFAULT_GOAL := all
.PHONY: all clean

# Include dependencies
-include $(DEPS)
