-include Makefile.local

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

INCDIR += $(ROCKSDB_INCDIR)
LDDIR += $(ROCKSDB_LDDIR)

##############################################################################
# MKL configuration
MKL_INCDIR += -I/opt/intel/mkl/include
MKL_LDDIR += -L/opt/intel/mkl/lib/intel64 -Wl,-rpath=/opt/intel/mkl/lib/intel64
MKL_LDLIBS += -lmkl_rt

INCDIR += $(MKL_INCDIR)
LDDIR += $(MKL_LDDIR)
LDLIBS += $(MKL_LDLIBS)

##############################################################################

CPPFLAGS += $(INCDIR)
CPPFLAGS += $(EXTRA_CPPFLAGS)
LDFLAGS += -no-pie -pthread -g $(LDDIR)
LDFLAGS += $(EXTRA_LDFLAGS)

##############################################################################

include mk/recipes.mk

DEPS :=
CLEAN :=
TARGETS := prepare localtest split_workload ycsb_remote_client ycsb_loader ycsb_server switchos controller dpdktest_client dpdktest_server socktest_client socktest_server skewness_check

DEPS += dpdk_helper.d
CLEAN += dpdk_helper.o

DEPS += key.d val.d
CLEAN += key.o val.o

DEPS += ycsb/parser.d
CLEAN += ycsb/parser.o

DEPS += crc32.d
CLEAN += crc32.o

DEPS += socket_helper.d
CLEAN += socket_helper.o

#DEPS += backup_data.d special_case.d
#CLEAN += backup_data.d backup_data.o special_case.d special_case.o
#BACKUP_OBJECTS := backup_data.o special_case.o
DEPS += special_case.d
CLEAN += special_case.o

DEPS += latency_helper.d
CLEAN += latency_helper.o

DEPS += rocksdb_wrapper.d
CLEAN += rocksdb_wrapper.o

INIPARSER_OBJECTS := iniparser/iniparser_wrapper.o iniparser/iniparser.o iniparser/dictionary.o
DEPS += iniparser/iniparser.d iniparser/dictionary.d iniparser/iniparser_wrapper.d
CLEAN += iniparser/iniparser.o iniparser/dictionary.o iniparser/iniparser_wrapper.o

DEPS += snapshot_record.d
CLEAN += snapshot_record.o

prepare: prepare.o key.o val.o
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

localtest: localtest.o key.o val.o ycsb/parser.o $(INIPARSER_OBJECTS) rocksdb_wrapper.o snapshot_record.o
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += localtest.d
CLEAN += localtest.o localtest

split_workload: split_workload.o key.o val.o ycsb/parser.o $(INIPARSER_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += split_workload.d
CLEAN += split_workload.o split_workload

ycsb_loader: ycsb_loader.o key.o val.o ycsb/parser.o $(INIPARSER_OBJECTS) rocksdb_wrapper.o snapshot_record.o
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += ycsb_loader.d
CLEAN += ycsb_loader.o ycsb_loader

ycsb_remote_client: ycsb_remote_client.o dpdk_helper.o key.o val.o ycsb/parser.o $(INIPARSER_OBJECTS) crc32.o latency_helper.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += ycsb_remote_client.d
CLEAN += ycsb_remote_client.o ycsb_remote_client

ycsb_server: ycsb_server.o dpdk_helper.o key.o val.o ycsb/parser.o socket_helper.o $(INIPARSER_OBJECTS) latency_helper.o rocksdb_wrapper.o snapshot_record.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += ycsb_server.d
CLEAN += ycsb_server.o ycsb_server

switchos: switchos.o key.o val.o socket_helper.o $(INIPARSER_OBJECTS) special_case.o
	$(CC) -pthread -g $(LDDIR) $^ -lpthread -o $@
DEPS += switchos.d
CLEAN += switchos.o switchos

controller: controller.o key.o val.o socket_helper.o $(INIPARSER_OBJECTS) ycsb/parser.o
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += controller.d
CLEAN += controller.o controller

dpdktest_client: dpdktest_client.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o ycsb/parser.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += dpdktest_client.d
CLEAN += dpdktest_client.o dpdktest_client

dpdktest_server: dpdktest_server.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o ycsb/parser.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += dpdktest_server.d
CLEAN += dpdktest_server.o dpdktest_server

socktest_client: socktest_client.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o socket_helper.o ycsb/parser.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += socktest_client.d
CLEAN += socktest_client.o socktest_client

socktest_server: socktest_server.o dpdk_helper.o key.o val.o $(INIPARSER_OBJECTS) latency_helper.o socket_helper.o ycsb/parser.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += socktest_server.d
CLEAN += socktest_server.o socktest_server

skewness_check: skewness_check.o  key.o val.o ycsb/parser.o $(INIPARSER_OBJECTS)
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
