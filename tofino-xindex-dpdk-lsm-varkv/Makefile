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

#ROCKSDB_INCDIR += -I../rocksdb-6.22.1/include
#ROCKSDB_LDDIR += -L../rocksdb-6.22.1
ROCKSDB_INCDIR += -I../rocksdb-6.22.1-model-v2/include
ROCKSDB_LDDIR += -L../rocksdb-6.22.1-model-v2
ROCKSDB_LDLIBS = -lrocksdb -lzstd -lbz2 -llz4 -lsnappy -lz

INCDIR += $(ROCKSDB_INCDIR)
LDDIR += $(ROCKSDB_LDDIR)
LDLIBS += $(ROCKSDB_LDLIBS)

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
LDFLAGS += -pthread -g $(LDDIR)
LDFLAGS += $(EXTRA_LDFLAGS)

##############################################################################

include mk/recipes.mk

DEPS :=
CLEAN :=
TARGETS := client server prepare localtest

DEPS += dpdk_helper.d
CLEAN += dpdk_helper.o

DEPS += key.d
CLEAN += key.o

prepare: prepare.o key.o
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += prepare.d
CLEAN += prepare.o prepare

client: client.o dpdk_helper.o key.o
	echo $(LDFLAGS)
	echo $(LDLIBS)
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += client.d
CLEAN += client.o client

server: server.o dpdk_helper.o key.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += server.d
CLEAN += server.o serer

localtest: localtest.o key.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += localtest.d
CLEAN += localtest.o localtest

##############################################################################
# Top level targets

all: $(TARGETS)

clean:
	rm -rf $(CLEAN) $(DEPS)

.DEFAULT_GOAL := all
.PHONY: all clean

# Include dependencies
-include $(DEPS)
