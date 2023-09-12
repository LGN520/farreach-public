source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachlimitspine -t ${SWITCH_ROOTPATH}/distfarreachlimit/tofino-spine/recover_switch/ --target hw --setup
