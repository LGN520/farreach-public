#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbufferv4 -t /home/ssy/NetBuffer/farreach/tofino/ptf_popserver/ --target hw --setup
