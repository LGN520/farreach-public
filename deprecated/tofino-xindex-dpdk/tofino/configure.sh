source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p xindex -t /home/${USER}/${SWITCH_ROOTPATH}/tofino-xindex-dpdk/tofino/ --target hw --setup
