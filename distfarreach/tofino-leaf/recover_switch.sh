source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachleaf -t /home/${USER}/${CLIENT_ROOTPATH}/distfarreach/tofino-leaf/recover_switch/ --target hw --setup
