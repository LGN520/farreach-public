source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p nocache -t ${SWITCH_ROOTPATH}/nocache/tofino/configure/ --target hw --setup
