source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachlimitspine -t /home/${USER}/${SWITCH_ROOTPATH}/distfarreachlimit/tofino-spine/ptf_snapshotserver/ --target hw --setup
