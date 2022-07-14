#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachleaf -t /home/ssy/NetBuffer/distfarreach/tofino-leaf/recover_switch/ --target hw --setup
