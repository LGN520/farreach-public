-include Makefile.local

include ../common/mk/recipes.mk

DEPS :=
CLEAN :=

##############################################################################
# Compile, link, and install flags

CC := g++
#CFLAGS += -std=c++14 -O3 -g -Wall -Werror -march=native -fno-omit-frame-pointer
CFLAGS += -std=c++14 -O3 -g -Wall -march=native -fno-omit-frame-pointer
CFLAGS += $(EXTRA_CFLAGS)
CFLAGS_SHARED += $(CFLAGS) -fPIC
LDLIBS += -lpthread -ldl -lstdc++fs -lboost_system -lcommon
#-ljemalloc 
LDLIBS += $(EXTRA_LDLIBS)

LDDIR += -L/usr/lib/x86_64-linux-gnu -L../common
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
#ROCKSDB_LDLIBS = -lrocksdb_debug -lzstd -lbz2 -llz4 -lsnappy -lz -ldl

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

#DEPS += checksum_helper.d
#CLEAN += checksum_helper.o
#UTILS_OBJECTS += checksum_helper.o

#DEPS += seq_cache.d
#CLEAN += seq_cache.o
#UTILS_OBJECTS += seq_cache.o

##############################################################################

CPPFLAGS += $(INCDIR)
CPPFLAGS += $(EXTRA_CPPFLAGS)
LDFLAGS += -no-pie -pthread -g $(LDDIR)
LDFLAGS += $(EXTRA_LDFLAGS)

##############################################################################

TARGETS := 

prepare: prepare.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += prepare.d
CLEAN += prepare.o
TARGETS += prepare

localtest: localtest.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += localtest.d
CLEAN += localtest.o
TARGETS += localtest

split_workload: split_workload.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += split_workload.d
CLEAN += split_workload.o
TARGETS += split_workload

loader: loader.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += loader.d
CLEAN += loader.o
TARGETS += loader

warmup_client: warmup_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += warmup_client.d
CLEAN += warmup_client.o
TARGETS += warmup_client

remote_client: remote_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += remote_client.d
CLEAN += remote_client.o
TARGETS += remote_client

#server: server.o $(UTILS_OBJECTS) $(ROCKSDB_OBJECTS) $(DPDK_OBJECTS)
	#$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) $(DPDK_LDLIBS) -o $@
server: server.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += server.d
CLEAN += server.o
TARGETS += server

switchos: switchos.o $(UTILS_OBJECTS)
	$(CC) -pthread -g $(LDDIR) $^ -lpthread -lcommon -o $@
DEPS += switchos.d
CLEAN += switchos.o
TARGETS += switchos

controller: controller.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += controller.d
CLEAN += controller.o
TARGETS += controller

#dpdktest_client: dpdktest_client.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_client.d
#CLEAN += dpdktest_client.o
#TARGETS += dpdktest_client

#dpdktest_server: dpdktest_server.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_server.d
#CLEAN += dpdktest_server.o
#TARGETS += dpdktest_server

#socktest_client: socktest_client.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_client.d
#CLEAN += socktest_client.o
#TARGETS += socktest_client

#socktest_server: socktest_server.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_server.d
#CLEAN += socktest_server.o
#TARGETS += socktest_server

skewness_check: skewness_check.o $(UTILS_OBJECTS) 
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += skewness_check.d
CLEAN += skewness_check.o
TARGETS += skewness_check

##############################################################################
# Top level targets

all: $(TARGETS)
	rm -rf $(CLEAN) $(DEPS)

clean:
	rm -rf $(CLEAN) $(DEPS) $(TARGETS)

.DEFAULT_GOAL := all
.PHONY: all clean

# Include dependencies
-include $(DEPS)
