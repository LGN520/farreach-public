source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachlimitspine -t /home/${USER}/${CLIENT_ROOTPATH}/distfarreachlimit/tofino-spine/configure/ --target hw --setup
