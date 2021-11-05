#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p resubmit -t /home/ssy/NetBuffer/resubmit-trial/test/ --target hw --setup
