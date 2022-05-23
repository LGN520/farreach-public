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
LDLIBS += -lpthread -ldl -ljemalloc -lstdc++fs -lboost_system
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

UTILS_OBJETS :=

DEPS += key.d val.d
CLEAN += key.o val.o
UTILS_OBJETS += key.o val.o

DEPS += socket_helper.d checksum_helper.d
CLEAN += socket_helper.o checksum_helper.o
UTILS_OBJETS += socket_helper.o checksum_helper.o

DEPS += ycsb/parser.d
CLEAN += ycsb/parser.o
UTILS_OBJETS += ycsb/parser.o

DEPS += crc32.d
CLEAN += crc32.o
UTILS_OBJETS += crc32.o

DEPS += iniparser/iniparser.d iniparser/dictionary.d iniparser/iniparser_wrapper.d
CLEAN += iniparser/iniparser.o iniparser/dictionary.o iniparser/iniparser_wrapper.o
UTILS_OBJETS += iniparser/iniparser_wrapper.o iniparser/iniparser.o iniparser/dictionary.o

DEPS += special_case.d
CLEAN += special_case.o
UTILS_OBJETS += special_case.o

DEPS += snapshot_record.d
CLEAN += snapshot_record.o
UTILS_OBJETS += snapshot_record.o

DEPS += latency_helper.d
CLEAN += latency_helper.o
UTILS_OBJETS += latency_helper.o

##############################################################################

CPPFLAGS += $(INCDIR)
CPPFLAGS += $(EXTRA_CPPFLAGS)
LDFLAGS += -no-pie -pthread -g $(LDDIR)
LDFLAGS += $(EXTRA_LDFLAGS)

##############################################################################

TARGETS := prepare localtest split_workload ycsb_remote_client ycsb_loader ycsb_server switchos controller skewness_check
#dpdktest_client dpdktest_server socktest_client socktest_server
##client server

prepare: prepare.o $(UTILS_OBJETS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += prepare.d
CLEAN += prepare.o prepare

#client: client.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS)
#	echo $(LDFLAGS)
#	echo $(LDLIBS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += client.d
#CLEAN += client.o client

#server: server.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += server.d
#CLEAN += server.o server

localtest: localtest.o $(UTILS_OBJETS) $(ROCKSDB_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += localtest.d
CLEAN += localtest.o localtest

split_workload: split_workload.o $(UTILS_OBJETS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += split_workload.d
CLEAN += split_workload.o split_workload

ycsb_loader: ycsb_loader.o $(UTILS_OBJETS) $(ROCKSDB_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += ycsb_loader.d
CLEAN += ycsb_loader.o ycsb_loader

ycsb_remote_client: ycsb_remote_client.o $(UTILS_OBJETS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += ycsb_remote_client.d
CLEAN += ycsb_remote_client.o ycsb_remote_client

#ycsb_server: ycsb_server.o $(UTILS_OBJETS) $(ROCKSDB_OBJECTS) $(DPDK_OBJECTS)
	#$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) $(DPDK_LDLIBS) -o $@
ycsb_server: ycsb_server.o $(UTILS_OBJETS) $(ROCKSDB_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += ycsb_server.d
CLEAN += ycsb_server.o ycsb_server

switchos: switchos.o $(UTILS_OBJETS)
	$(CC) -pthread -g $(LDDIR) $^ -lpthread -o $@
DEPS += switchos.d
CLEAN += switchos.o switchos

controller: controller.o $(UTILS_OBJETS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += controller.d
CLEAN += controller.o controller

#dpdktest_client: dpdktest_client.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o ycsb/parser.o
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_client.d
#CLEAN += dpdktest_client.o dpdktest_client

#dpdktest_server: dpdktest_server.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o ycsb/parser.o
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_server.d
#CLEAN += dpdktest_server.o dpdktest_server

#socktest_client: socktest_client.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o socket_helper.o ycsb/parser.o
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_client.d
#CLEAN += socktest_client.o socktest_client

#socktest_server: socktest_server.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o socket_helper.o ycsb/parser.o
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_server.d
#CLEAN += socktest_server.o socktest_server

skewness_check: skewness_check.o $(UTILS_OBJETS) 
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += skewness_check.d
CLEAN += skewness_check.o skewness_check

##############################################################################
# Top level targets

all: $(TARGETS)

clean:
	rm -rf $(CLEAN) $(DEPS)

.DEFAULT_GOAL := all
.PHONY: all clean

# Include dependencies
-include $(DEPS)
