#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreach -t /home/ssy/NetBuffer/distfarreach/tofino/recover_switch/ --target hw --setup
