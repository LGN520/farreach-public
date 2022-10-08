source ../../scripts/common.sh
ssh ssy@bf2 "rm -rf /home/${USER}/${SWITCH_ROOTPATH}/netreach-v4/tofino"
scp -r ../tofino ssy@bf2:/home/${USER}/${SWITCH_ROOTPATH}/netreach-v4/tofino
