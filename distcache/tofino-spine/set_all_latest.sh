#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distcache/tofino-spine

cd $SDE
./run_p4_tests.sh -p distcachespine -t ${SWITCH_ROOTPATH}/distcache/tofino-spine/set_all_latest/ --target hw --setup
