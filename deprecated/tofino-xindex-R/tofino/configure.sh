source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p xindex -t /home/${USER}/${CLIENT_ROOTPATH}/tofino-xindex-R/tofino/ --target hw --setup
