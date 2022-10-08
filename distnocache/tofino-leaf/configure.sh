source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distnocacheleaf -t /home/${USER}/${CLIENT_ROOTPATH}/distnocache/tofino-leaf/configure/ --target hw --setup
