source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p netbuffer -t /home/${USER}/${SWITCH_ROOTPATH}/tofino-netbuffer-dpdk-lsm/tofino/configure/ --target hw --setup
