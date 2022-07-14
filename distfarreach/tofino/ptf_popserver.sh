#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreach -t /home/ssy/NetBuffer/distfarreach/tofino/ptf_popserver/ --target hw --setup
