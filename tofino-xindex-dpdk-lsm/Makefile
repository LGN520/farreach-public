-include Makefile.local

##############################################################################
# Compile, link, and install flags

CC := g++
CPPFLAGS += -Iinclude/
CPPFLAGS += $(EXTRA_CPPFLAGS)
#CFLAGS += -std=c++14 -O3 -g -Wall -Werror -march=native -fno-omit-frame-pointer
CFLAGS += -std=c++14 -O3 -g -Wall -march=native -fno-omit-frame-pointer
CFLAGS += $(EXTRA_CFLAGS)
CFLAGS_SHARED += $(CFLAGS) -fPIC
LDFLAGS += -pthread -g
LDFLAGS += $(EXTRA_LDFLAGS)
LDLIBS += -lpthread -ldl -ljemalloc
LDLIBS += $(EXTRA_LDLIBS)

PREFIX ?= /usr/local
SBINDIR ?= $(PREFIX)/sbin
LIBDIR ?= $(PREFIX)/lib
INCDIR ?= $(PREFIX)/include

##############################################################################
# MKL configuration
MKL_CPPFLAGS += -I/opt/intel/mkl/include
MKL_LDFLAGS += -L/opt/intel/mkl/lib/intel64
MKL_LDLIBS += -lmkl_rt

CPPFLAGS += $(MKL_CPPFLAGS)
LDFLAGS += $(MKL_LDFLAGS)
LDLIBS += $(MKL_LDLIBS)

##############################################################################
# DPDK configuration

# Prefix for dpdk
#RTE_SDK ?= /usr/
# mpdts to compile
DPDK_PMDS ?= ixgbe i40e tap virtio

DPDK_CPPFLAGS += -I$(RTE_SDK)/$(RTE_TARGET)/include
DPDK_LDFLAGS += -L$(RTE_SDK)/$(RTE_TARGET)/lib/
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

CPPFLAGS += $(DPDK_CPPFLAGS)
LDFLAGS += $(DPDK_LDFLAGS)

##############################################################################
# RocksDB configuration

ROCKSDB_CPPFLAGS += -I../rocksdb-6.22.1/include
ROCKSDB_LDFLAGS += -L../rocksdb-6.22.1
ROCKSDB_LDLIBS = -lrocksdb -lzstd -lbz2 -llz4 -lsnappy -lz

CPPFLAGS += $(ROCKSDB_CPPFLAGS)
LDFLAGS += $(ROCKSDB_LDFLAGS)

##############################################################################

include mk/recipes.mk

DEPS :=
CLEAN :=
TARGETS := client server prepare

DEPS += dpdk_helper.d
CLEAN += dpdk_helper.o

prepare: prepare.o
	$(LINK) $^ $(LDLIBS) -o $@
DEPS += prepare.d
CLEAN += prepare.o prepare

client: client.o dpdk_helper.o
	echo $(LDFLAGS)
	echo $(LDLIBS)
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) -o $@
DEPS += client.d
CLEAN += client.o client

server: server.o dpdk_helper.o
	$(LINK) $^ $(LDLIBS) $(DPDK_LDLIBS) $(ROCKSDB_LDLIBS) -o $@
DEPS += server.d
CLEAN += server.o serer

##############################################################################
# Top level targets

all: $(TARGETS)

clean:
	rm -rf $(CLEAN) $(DEPS)



.DEFAULT_GOAL := all
.PHONY: all clean

# Include dependencies
-include $(DEPS)
