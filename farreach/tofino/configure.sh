#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd farreach/tofino

source /root/.bashrc

$SDE/run_p4_tests.sh -p netbufferv4 -t ${SWITCH_ROOTPATH}/farreach/tofino/configure/ --target hw --setup

# bfrt_python ./setup_61_165.py true
