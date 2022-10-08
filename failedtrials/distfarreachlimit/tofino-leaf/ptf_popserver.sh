source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachlimitleaf -t /home/${USER}/${SWITCH_ROOTPATH}/distfarreachlimit/tofino-leaf/ptf_popserver/ --target hw --setup
