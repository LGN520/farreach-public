#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distcache/tofino-leaf

cd $SDE
./run_p4_tests.sh -p distcacheleaf -t ${SWITCH_ROOTPATH}/distcache/tofino-leaf/configure/ --target hw --setup
