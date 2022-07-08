#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netcache -t /home/ssy/NetBuffer/netcache/tofino/configure/ --target hw --setup
