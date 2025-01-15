-include Makefile.local

include ../common/mk/recipes.mk

DEPS :=
CLEAN :=

##############################################################################
# Compile, link, and install flags

CC := g++
#CFLAGS += -std=c++14 -O3 -g -Wall -Werror -march=native -fno-omit-frame-pointer
CFLAGS += -std=c++14 -O0 -g -Wall -march=native -fno-omit-frame-pointer
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


simu_failure: simu_failure.c $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += simu_failure.d
CLEAN += simu_failure.o
CLIENT_TARGETS += simu_failure
simu_recover_load_persist: simu_recover_load_persist.c $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += simu_recover_load_persist.d
CLEAN += simu_recover_load_persist.o
CLIENT_TARGETS += simu_recover_load_persist
simu_recover_prepare: simu_recover_prepare.c $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += simu_recover_prepare.d
CLEAN += simu_recover_prepare.o
CLIENT_TARGETS += simu_recover_prepare
# simu_recover_server: simu_recover_server.c $(UTILS_OBJECTS)
# 	$(LINK) $^ $(LDLIBS) -o $@
# DEPS += simu_recover_server.d
# CLEAN += simu_recover_server.o
# CLIENT_TARGETS += simu_recover_server
simu_recover_switch: simu_recover_switch.c $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += simu_recover_switch.d
CLEAN += simu_recover_switch.o
CLIENT_TARGETS += simu_recover_switch

warmup_client: warmup_client.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += warmup_client.d
CLEAN += warmup_client.o
CLIENT_TARGETS += warmup_client

switchos: switchos.o $(UTILS_OBJECTS)
	$(CC) -pthread -g $(LDDIR) $^ -lpthread -lcommon -o $@
DEPS += switchos.d
CLEAN += switchos.o
SWITCH_TARGETS += switchos

server: server.o $(ROCKSDB_OBJECTS) $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) $(ROCKSDB_LDLIBS) -o $@

DEPS += server.d
CLEAN += server.o
SERVER_TARGETS += server

controller: controller.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += controller.d
CLEAN += controller.o
SERVER_TARGETS += controller


recover_tofino: recover_tofino.o $(UTILS_OBJECTS)
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += recover_tofino.d
CLEAN += recover_tofino.o
SERVER_TARGETS += recover_tofino

# 1: 1.o $(UTILS_OBJECTS)
# 	$(LINK) $^ $(LDLIBS) -o $@
# DEPS += 1.d
# CLEAN += 1.o
# SERVER_TARGETS += 1
##############################################################################
# Top level targets

all: $(CLIENT_TARGETS) $(SWITCH_TARGETS)  $(SERVER_TARGETS)
	rm -rf $(CLEAN) $(DEPS)

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
