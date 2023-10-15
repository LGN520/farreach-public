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
LDLIBS += -lpthread -ldl -lstdc++fs -lboost_system -lboost_thread -lcommon
#-ljemalloc 
LDLIBS += $(EXTRA_LDLIBS)

LDDIR += -L../boost_1_81_0/install/lib -L/usr/lib/x86_64-linux-gnu -L../common
INCDIR += -I../boost_1_81_0/install/include -I/usr/include

##############################################################################
## DPDK configuration
#
## Prefix for dpdk
##RTE_SDK ?= /usr/
## mpdts to compile
#DPDK_PMDS ?= ixgbe i40e tap virtio
#
#DPDK_INCDIR += -I$(RTE_SDK)/$(RTE_TARGET)/include
#DPDK_LDDIR += -L$(RTE_SDK)/$(RTE_TARGET)/lib/
#DPDK_LDLIBS += \
#  -Wl,--whole-archive \
#   $(addprefix -lrte_pmd_,$(DPDK_PMDS)) \
#  -lrte_eal \
#  -lrte_mempool \
#  -lrte_mempool_ring \
#  -lrte_hash \
#  -lrte_ring \
#  -lrte_kvargs \
#  -lrte_ethdev \
#  -lrte_mbuf \
#  -lnuma \
#  -lrte_bus_pci \
#  -lrte_pci \
#  -lrte_cmdline \
#  -lrte_timer \
#  -lrte_net \
#  -lrte_kni \
#  -lrte_bus_vdev \
#  -lrte_gso \
#  -Wl,--no-whole-archive \
#  -ldl \
#  $(EXTRA_LIBS_DPDK)
#
#DEPS += dpdk_helper.d
#CLEAN += dpdk_helper.o
#DPDK_OBJECTS += dpdk_helper.o
#
#INCDIR += $(DPDK_INCDIR)
#LDDIR += $(DPDK_LDDIR)

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
# TommyDS configuration

TOMMYDS_INCDIR += -I../tommyds-2.2/tommyds
TOMMYDS_LDDIR += -L../tommyds-2.2

TOMMYDS_LDLIBS = -ltommy

INCDIR += $(TOMMYDS_INCDIR)
LDDIR += $(TOMMYDS_LDDIR)

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

DEPS += outband_packet_format.d
CLEAN += outband_packet_format.d
UTILS_OBJECTS += outband_packet_format.o

##############################################################################

CPPFLAGS += $(INCDIR)
CPPFLAGS += $(EXTRA_CPPFLAGS)
LDFLAGS += -no-pie -pthread -g $(LDDIR)
LDFLAGS += $(EXTRA_LDFLAGS)

##############################################################################

CLIENT_TARGETS :=
SWITCH_TARGETS :=
SERVER_TARGETS := 

prepare: prepare.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += prepare.d
CLEAN += prepare.o
CLIENT_TARGETS += prepare

split_workload: split_workload.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += split_workload.d
CLEAN += split_workload.o
CLIENT_TARGETS += split_workload

warmup_client: warmup_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += warmup_client.d
CLEAN += warmup_client.o
CLIENT_TARGETS += warmup_client

#remote_client: remote_client.o $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += remote_client.d
#CLEAN += remote_client.o
#CLIENT_TARGETS += remote_client

preparefinish_client: preparefinish_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += preparefinish_client.d
CLEAN += preparefinish_client.o
CLIENT_TARGETS += preparefinish_client

switchos: switchos.o $(UTILS_OBJECTS)
	$(CC) -pthread -g $(LDDIR) $^ -lpthread -lcommon -o $@
DEPS += switchos.d
CLEAN += switchos.o
SWITCH_TARGETS += switchos

#localtest: localtest.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
#DEPS += localtest.d
#CLEAN += localtest.o
#SERVER_TARGETS += localtest

#loader: loader.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
#DEPS += loader.d
#CLEAN += loader.o
#SERVER_TARGETS += loader

##server: server.o $(UTILS_OBJECTS) $(ROCKSDB_OBJECTS) $(DPDK_OBJECTS)
##	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) $(DPDK_LDLIBS) -o $@
server: server.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
#server: server.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(TOMMYDS_LDLIBS) -o $@
DEPS += server.d
CLEAN += server.o
SERVER_TARGETS += server

controller: controller.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += controller.d
CLEAN += controller.o
SERVER_TARGETS += controller

#dpdktest_client: dpdktest_client.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_client.d
#CLEAN += dpdktest_client.o
#CLIENT_TARGETS += dpdktest_client

#dpdktest_server: dpdktest_server.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
#DEPS += dpdktest_server.d
#CLEAN += dpdktest_server.o
#SERVER_TARGETS += dpdktest_server

#socktest_client: socktest_client.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_client.d
#CLEAN += socktest_client.o
#CLIENT_TARGETS += socktest_client

#socktest_server: socktest_server.o $(DPDK_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) -o $@
#DEPS += socktest_server.d
#CLEAN += socktest_server.o
#SERVER_TARGETS += socktest_server

skewness_check: skewness_check.o $(UTILS_OBJECTS) 
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += skewness_check.d
CLEAN += skewness_check.o
CLIENT_TARGETS += skewness_check

# recover/controller_get_snapshotid: recover/controller_get_snapshotid.o $(UTILS_OBJECTS)
# 	$(LINK) $^ $(LDLIBS) -o $@
# DEPS += recover/controller_get_snapshotid.d
# CLEAN += recover/controller_get_snapshotid.o
# SERVER_TARGETS += recover/controller_get_snapshotid

#recover/server_recover: recover/server_recover.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
#	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@
#DEPS += recover/server_recover.d
#CLEAN += recover/server_recover.o
#SERVER_TARGETS += recover/server_recover

##############################################################################
# Top level targets

#all: $(TARGETS)
#	rm -rf $(CLEAN) $(DEPS)

allclient: $(CLIENT_TARGETS)
	rm -rf $(CLEAN) $(DEPS)

allswitch: $(SWITCH_TARGETS)
	rm -rf $(CLEAN) $(DEPS)

allserver: $(SERVER_TARGETS)
	rm -rf $(CLEAN) $(DEPS)

clean:
	rm -rf $(CLEAN) $(DEPS) $(CLINET_TARGETS) $(SWITCH_TARGETS) $(SERVER_TARGETS)

.DEFAULT_GOAL := all
.PHONY: all clean

# Include dependencies
-include $(DEPS)
