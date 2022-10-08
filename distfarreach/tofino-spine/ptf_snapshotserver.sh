source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachspine -t /home/${USER}/${SWITCH_ROOTPATH}/distfarreach/tofino-spine/ptf_snapshotserver/ --target hw --setup
