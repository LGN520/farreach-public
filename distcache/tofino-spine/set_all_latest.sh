#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distcachespine -t /home/ssy/NetBuffer/distcache/tofino-spine/set_all_latest/ --target hw --setup
