set -x
source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbufferv4 -t ${SWITCH_ROOTPATH}/netreach-v4-lsm/tofino/reset_snapshot_flag_and_reg/ --target hw --setup
