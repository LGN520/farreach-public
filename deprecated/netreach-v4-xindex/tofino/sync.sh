source ../../scripts/common.sh
ssh ${USER}@bf2 "rm -rf /home/${USER}/${SWITCH_ROOTPATH}/netreach-v4/tofino"
scp -r ../tofino ${USER}@bf2:/home/${USER}/${SWITCH_ROOTPATH}/netreach-v4/tofino
