source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netcache -t /home/${USER}/${CLIENT_ROOTPATH}/netcache/tofino/ptf_cleaner/ --target hw --setup
