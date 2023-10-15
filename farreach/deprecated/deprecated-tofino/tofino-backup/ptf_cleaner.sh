set -x
#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd farreach/tofino

cd $SDE
./run_p4_tests.sh -p netbufferv4 -t ${SWITCH_ROOTPATH}/farreach/tofino/ptf_cleaner/ --target hw --setup
