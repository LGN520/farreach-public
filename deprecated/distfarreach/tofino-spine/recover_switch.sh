#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distfarreach/tofino-spine

cd $SDE
./run_p4_tests.sh -p distfarreachspine -t ${SWITCH_ROOTPATH}/distfarreach/tofino-spine/recover_switch/ --target hw --setup
