CC := g++
CFLAGS += -std=c++14 -O3
CFLAGS_SHARED += $(CFLAGS) -fPIC

#JAVA_HOME = "/usr/lib/jvm/java-8-openjdk-amd64/"
INCDIR := -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux

LDDIR := -L../../../common
LDLIBS := -lcommon_shared

CPPFLAGS += $(INCDIR)

SocketJNI.shared.o: com_inswitchcache_core_SocketHelper.c
	$(CC) $(CFLAGS_SHARED) $(CPPFLAGS) -c $^ -o $@

libSocketJNI.so: SocketJNI.shared.o
	$(CC) -shared $^ $(LDDIR) $(LDLIBS) -o $@
	mkdir -p ../../ycsb/jnilib
	mv libSocketJNI.so ../../ycsb/jnilib
	rm SocketJNI.shared.o

all: libSocketJNI.so

.PHONY: clean
clean:
	rm -r libSocketJNI.so SocketJNI.shared.o
