#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distreach/tofino

source /root/.bashrc

$SDE/run_p4_tests.sh -p distreach -t ${SWITCH_ROOTPATH}/distreach/tofino/configure/ --target hw --setup

# bfrt_python ./setup_61_165.py true
