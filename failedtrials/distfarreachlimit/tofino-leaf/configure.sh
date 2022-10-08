source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distfarreachlimitleaf -t /home/${USER}/${CLIENT_ROOTPATH}/distfarreachlimit/tofino-leaf/configure/ --target hw --setup
