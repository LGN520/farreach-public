source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p socktest -t /home/${USER}/${CLIENT_ROOTPATH}/netreach-v4/socktest-tofino/ --target hw --setup
