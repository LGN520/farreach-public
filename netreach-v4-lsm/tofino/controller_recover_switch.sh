#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbufferv4 -t /home/ssy/NetBuffer/netreach-v4-lsm/tofino/controller_recover_switch/ --target hw --setup
