source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distcacheleaf -t /home/${USER}/${CLIENT_ROOTPATH}/distcache/tofino-leaf/configure/ --target hw --setup
