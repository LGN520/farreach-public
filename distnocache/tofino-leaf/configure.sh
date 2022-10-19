#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distnocache/tofino-leaf

cd $SDE
./run_p4_tests.sh -p distnocacheleaf -t ${SWITCH_ROOTPATH}/distnocache/tofino-leaf/configure/ --target hw --setup
