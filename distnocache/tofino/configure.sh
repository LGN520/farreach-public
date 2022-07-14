#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distnocache -t /home/ssy/NetBuffer/distnocache/tofino/configure/ --target hw --setup
