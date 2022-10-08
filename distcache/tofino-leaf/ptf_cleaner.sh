source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distcacheleaf -t /home/${USER}/${SWITCH_ROOTPATH}/distcache/tofino-leaf/ptf_cleaner/ --target hw --setup
