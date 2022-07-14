#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachspine -t /home/ssy/NetBuffer/distfarreach/tofino-spine/recover_switch/ --target hw --setup
