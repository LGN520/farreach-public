source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p dpdktest -t /home/${USER}/${CLIENT_ROOTPATH}/netreach-v4/dpdktest-tofino/ --target hw --setup
