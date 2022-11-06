#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd netcache/tofino

source /root/.zshrc

cd $SDE
./run_p4_tests.sh -p netcache -t ${SWITCH_ROOTPATH}/netcache/tofino/ptf_popserver/ --target hw --setup
