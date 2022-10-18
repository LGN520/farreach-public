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
INCDIR += $(ROCKSDB_INCDIR)

# TommyDS configuration
TOMMYDS_INCDIR += -I../tommyds-2.2/tommyds
INCDIR += $(TOMMYDS_INCDIR)

DEPS += rocksdb_wrapper.d
CLEAN += rocksdb_wrapper.o rocksdb_wrapper.shared.o
ROCKSDB_OBJECTS += rocksdb_wrapper.o
ROCKSDB_SHARED_OBJECTS += rocksdb_wrapper.shared.o

##############################################################################

# Utils

UTILS_OBJECTS :=

DEPS += key.d val.d
CLEAN += key.o val.o key.shared.o val.shared.o
UTILS_OBJECTS += key.o val.o
UTILS_SHARED_OBJECTS += key.shared.o val.shared.o

DEPS += crc32.d
CLEAN += crc32.o crc32.shared.o
UTILS_OBJECTS += crc32.o
UTILS_SHARED_OBJECTS += crc32.shared.o

DEPS += dynamic_array.d
CLEAN += dynamic_array.o dynamic_array.shared.o
UTILS_OBJECTS += dynamic_array.o
UTILS_SHARED_OBJECTS += dynamic_array.shared.o

DEPS += workloadparser/parser.d workloadparser/ycsb_parser.d workloadparser/synthetic_parser.d
CLEAN += workloadparser/parser.o workloadparser/ycsb_parser.o workloadparser/synthetic_parser.o workloadparser/parser.shared.o workloadparser/ycsb_parser.shared.o workloadparser/synthetic_parser.shared.o
UTILS_OBJECTS += workloadparser/parser.o workloadparser/ycsb_parser.o workloadparser/synthetic_parser.o
UTILS_SHARED_OBJECTS += workloadparser/parser.shared.o workloadparser/ycsb_parser.shared.o workloadparser/synthetic_parser.shared.o

DEPS += iniparser/iniparser.d iniparser/dictionary.d iniparser/iniparser_wrapper.d
CLEAN += iniparser/iniparser.o iniparser/dictionary.o iniparser/iniparser_wrapper.o iniparser/iniparser.shared.o iniparser/dictionary.shared.o iniparser/iniparser_wrapper.shared.o
UTILS_OBJECTS += iniparser/iniparser_wrapper.o iniparser/iniparser.o iniparser/dictionary.o
UTILS_SHARED_OBJECTS += iniparser/iniparser_wrapper.shared.o iniparser/iniparser.shared.o iniparser/dictionary.shared.o

DEPS += special_case.d
CLEAN += special_case.o special_case.shared.o
UTILS_OBJECTS += special_case.o
UTILS_SHARED_OBJECTS += special_case.shared.o

DEPS += snapshot_record.d
CLEAN += snapshot_record.o snapshot_record.shared.o
UTILS_OBJECTS += snapshot_record.o
UTILS_SHARED_OBJECTS += snapshot_record.shared.o

DEPS += latency_helper.d
CLEAN += latency_helper.o latency_helper.shared.o
UTILS_OBJECTS += latency_helper.o
UTILS_SHARED_OBJECTS += latency_helper.shared.o

DEPS += dynamic_rulemap.d
CLEAN += dynamic_rulemap.o dynamic_rulemap.shared.o
UTILS_OBJECTS += dynamic_rulemap.o
UTILS_SHARED_OBJECTS += dynamic_rulemap.shared.o

DEPS += pkt_ring_buffer.d
CLEAN += pkt_ring_buffer.o pkt_ring_buffer.shared.o
UTILS_OBJECTS += pkt_ring_buffer.o
UTILS_SHARED_OBJECTS += pkt_ring_buffer.shared.o

DEPS += io_helper.d
CLEAN += io_helper.o io_helper.shared.o
UTILS_OBJECTS += io_helper.o
UTILS_SHARED_OBJECTS += io_helper.shared.o

DEPS += socket_helper.d
CLEAN += socket_helper.o socket_helper.shared.o
UTILS_OBJECTS += socket_helper.o
UTILS_SHARED_OBJECTS += socket_helper.shared.o

##############################################################################

CPPFLAGS += $(INCDIR)
CPPFLAGS += $(EXTRA_CPPFLAGS)

##############################################################################

TARGETS := libcommon.a libcommon_shared.a

libcommon.a: $(UTILS_OBJECTS) $(ROCKSDB_OBJECTS)
	ar -rcs $@ $^

libcommon_shared.a: $(UTILS_SHARED_OBJECTS) $(ROCKSDB_SHARED_OBJECTS)
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
