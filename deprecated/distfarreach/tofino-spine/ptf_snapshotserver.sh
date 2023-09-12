#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distfarreach/tofino-spine

source /root/.zshrc

cd $SDE
./run_p4_tests.sh -p distfarreachspine -t ${SWITCH_ROOTPATH}/distfarreach/tofino-spine/ptf_snapshotserver/ --target hw --setup
