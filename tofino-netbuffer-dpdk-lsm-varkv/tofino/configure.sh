#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/tofino-netbuffer-dpdk-lsm-varkv/tofino/configure/ --target hw --setup
