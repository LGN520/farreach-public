set -x
source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p socktest -t ${SWITCH_ROOTPATH}/netreach-v4/socktest-tofino/ --target hw --setup
