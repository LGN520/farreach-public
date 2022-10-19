#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd netcache/tofino

cd $SDE
./run_p4_tests.sh -p netcache -t ${SWITCH_ROOTPATH}/netcache/tofino/configure/ --target hw --setup
