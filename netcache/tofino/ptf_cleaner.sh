#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd nocache/tofino

source /root/.bashrc

$SDE/run_p4_tests.sh -p nocache_16 -t ${SWITCH_ROOTPATH}/nocache/tofino/ptf_cleaner/ --target hw --setup
# ./run_bfshell.sh -b ${SWITCH_ROOTPATH}/nocache/tofino/configure/table_configure.py

# bfrt_python ./setup_61_165.py true
