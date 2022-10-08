source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netcache -t /home/${USER}/${SWITCH_ROOTPATH}/netcache/tofino/configure/ --target hw --setup
