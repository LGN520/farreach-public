source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distcachespine -t ${SWITCH_ROOTPATH}/distcache/tofino-spine/ptf_cleaner/ --target hw --setup
