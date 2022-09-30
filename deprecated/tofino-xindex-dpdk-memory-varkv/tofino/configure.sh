#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p xindex -t /home/ssy/NetBuffer/tofino-xindex-dpdk-lsm-varkv/tofino/ --target hw --setup
