#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachlimitspine -t /home/ssy/NetBuffer/distfarreachlimit/tofino-spine/ptf_popserver/ --target hw --setup
