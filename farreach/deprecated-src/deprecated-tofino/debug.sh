source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t /home/${USER}/${SWITCH_ROOTPATH}/netreach-voting/tofino/debug/ --target hw --setup
