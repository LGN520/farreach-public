#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd nocache/tofino

cd $SDE
./run_p4_tests.sh -p nocache -t ${SWITCH_ROOTPATH}/nocache/tofino/configure/ --target hw --setup
