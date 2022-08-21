#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p dpdktest -t /home/ssy/NetBuffer/netreach-v4/dpdktest-tofino/ --target hw --setup
