source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbufferv4 -t /home/${USER}/${SWITCH_ROOTPATH}/netreach-v4-lsm/tofino/add_cache_lookup_setvalid1/ --target hw --setup
