source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t /home/${USER}/${CLIENT_ROOTPATH}/tofino-netbuffer-dpdk-lsm/tofino/debug/ --target hw --setup
