source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbufferv4 -t /home/${USER}/${CLIENT_ROOTPATH}/farreach/tofino/ptf_cleaner/ --target hw --setup
