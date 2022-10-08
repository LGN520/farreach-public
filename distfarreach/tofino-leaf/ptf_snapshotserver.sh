source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachleaf -t /home/${USER}/${SWITCH_ROOTPATH}/distfarreach/tofino-leaf/ptf_snapshotserver/ --target hw --setup
