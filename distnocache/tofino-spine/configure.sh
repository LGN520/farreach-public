#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distnocachespine -t /home/ssy/NetBuffer/distnocache/tofino-spine/configure/ --target hw --setup
