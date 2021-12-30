#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t /home/ssy/NetBuffer/netreach-voting/tofino/debug/ --target hw --setup
