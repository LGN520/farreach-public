source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t /home/${USER}/${CLIENT_ROOTPATH}/netreach/tofino/configure/ --target hw --setup
