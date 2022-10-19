#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distnocache/tofino-spine

cd $SDE
./run_p4_tests.sh -p distnocachespine -t ${SWITCH_ROOTPATH}/distnocache/tofino-spine/configure/ --target hw --setup
