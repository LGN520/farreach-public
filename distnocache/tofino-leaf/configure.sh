#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distnocacheleaf -t /home/ssy/NetBuffer/distnocache/tofino-leaf/configure/ --target hw --setup
