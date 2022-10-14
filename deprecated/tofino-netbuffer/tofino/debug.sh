source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t ${SWITCH_ROOTPATH}/tofino-netbuffer/tofino/debug/ --target hw --setup
