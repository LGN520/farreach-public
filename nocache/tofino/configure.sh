#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p nocache -t /home/ssy/NetBuffer/nocache/tofino/configure/ --target hw --setup
