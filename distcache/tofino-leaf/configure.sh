source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distcacheleaf -t ${SWITCH_ROOTPATH}/distcache/tofino-leaf/configure/ --target hw --setup
