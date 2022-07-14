#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distcacheleaf -t /home/ssy/NetBuffer/distcache/tofino-leaf/ptf_popserver/ --target hw --setup
