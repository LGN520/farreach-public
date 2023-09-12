#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd distfarreach/tofino-leaf

source /root/.zshrc

cd $SDE
./run_p4_tests.sh -p distfarreachleaf -t ${SWITCH_ROOTPATH}/distfarreach/tofino-leaf/ptf_snapshotserver/ --target hw --setup
