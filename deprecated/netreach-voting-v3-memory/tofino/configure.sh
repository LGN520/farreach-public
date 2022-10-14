source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbufferv3 -t ${SWITCH_ROOTPATH}/netreach-voting-v3/tofino/configure/ --target hw --setup
