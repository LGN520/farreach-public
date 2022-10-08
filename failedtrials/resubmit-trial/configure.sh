source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p resubmit -t /home/${USER}/${SWITCH_ROOTPATH}/resubmit-trial/test/ --target hw --setup
