#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach/tofino/configure/ --target hw --setup
