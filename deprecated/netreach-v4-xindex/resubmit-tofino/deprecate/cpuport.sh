#!/usr/bin/env bash

cd $SDE

if [ $1 == "setup" ]
then
	./run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/tofino-netbuffer-dpdk-lsm/tofino/cpuport/setup/ --target hw --setup
elif [ $1 == "cleanup" ]
then
	./run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/tofino-netbuffer-dpdk-lsm/tofino/cpuport/cleanup/ --target hw --setup
fi
