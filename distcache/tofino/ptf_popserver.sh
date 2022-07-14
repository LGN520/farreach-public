#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distcache -t /home/ssy/NetBuffer/distcache/tofino/ptf_popserver/ --target hw --setup
