source ../../scripts/common.sh
ssh ${USER}@bf2 "rm -rf ${SWITCH_ROOTPATH}/netreach-v4/tofino"
scp -r ../tofino ${USER}@bf2:${SWITCH_ROOTPATH}/netreach-v4/tofino
