source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE
./run_p4_tests.sh -p distnocachespine -t /home/${USER}/${SWITCH_ROOTPATH}/distnocache/tofino-spine/configure/ --target hw --setup
