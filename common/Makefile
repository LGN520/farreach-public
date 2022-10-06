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

INCDIR += -I/usr/include

##############################################################################
# RocksDB configuration

ROCKSDB_INCDIR += -I../rocksdb-6.22.1/include

DEPS += rocksdb_wrapper.d
CLEAN += rocksdb_wrapper.o
ROCKSDB_OBJECTS += rocksdb_wrapper.o

INCDIR += $(ROCKSDB_INCDIR)

##############################################################################

# Utils

UTILS_OBJECTS :=

DEPS += key.d val.d
CLEAN += key.o val.o
UTILS_OBJECTS += key.o val.o

DEPS += crc32.d
CLEAN += crc32.o
UTILS_OBJECTS += crc32.o

DEPS += dynamic_array.d
CLEAN += dynamic_array.o
UTILS_OBJECTS += dynamic_array.o

DEPS += workloadparser/parser.d workloadparser/ycsb_parser.d workloadparser/synthetic_parser.d
CLEAN += workloadparser/parser.o workloadparser/ycsb_parser.o workloadparser/synthetic_parser.o
UTILS_OBJECTS += workloadparser/parser.o workloadparser/ycsb_parser.o workloadparser/synthetic_parser.o

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

DEPS += dynamic_rulemap.d
CLEAN += dynamic_rulemap.o
UTILS_OBJECTS += dynamic_rulemap.o

DEPS += pkt_ring_buffer.d
CLEAN += pkt_ring_buffer.o
UTILS_OBJECTS += pkt_ring_buffer.o

DEPS += io_helper.d
CLEAN += io_helper.o
UTILS_OBJECTS += io_helper.o

DEPS += socket_helper.d
CLEAN += socket_helper.o
UTILS_OBJECTS += socket_helper.o

##############################################################################

CPPFLAGS += $(INCDIR)
CPPFLAGS += $(EXTRA_CPPFLAGS)

##############################################################################

TARGETS := libcommon.a

libcommon.a: $(UTILS_OBJECTS) $(ROCKSDB_OBJECTS)
	ar -rcs $@ $^

all: $(TARGETS)
	rm -rf $(CLEAN) $(DEPS)

##############################################################################

clean:
	rm -rf $(CLEAN) $(DEPS) $(TARGETS)

.DEFAULT_GOAL := all
.PHONY: all clean

# Include dependencies
-include $(DEPS)
