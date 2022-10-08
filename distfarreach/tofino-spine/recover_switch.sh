source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachspine -t /home/${USER}/${CLIENT_ROOTPATH}/distfarreach/tofino-spine/recover_switch/ --target hw --setup
