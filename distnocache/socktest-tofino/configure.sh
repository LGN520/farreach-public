#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p socktest -t /home/ssy/NetBuffer/netreach-v4/socktest-tofino/ --target hw --setup
