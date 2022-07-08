#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netcache -t /home/ssy/NetBuffer/netcache/tofino/ptf_cleaner/ --target hw --setup
