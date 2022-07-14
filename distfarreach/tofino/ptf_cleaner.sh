#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreach -t /home/ssy/NetBuffer/distfarreach/tofino/ptf_cleaner/ --target hw --setup
