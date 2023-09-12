#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distcache/tofino-spine

source /root/.zshrc

cd $SDE
./run_p4_tests.sh -p distcachespine -t ${SWITCH_ROOTPATH}/distcache/tofino-spine/ptf_popserver/ --target hw --setup
