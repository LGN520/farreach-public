source ../../scripts/common.sh
ssh ssy@bf2 "rm -rf /home/${USER}/${SWITCH_ROOTPATH}/netreach-voting-v3/tofino"
scp -r ../tofino ssy@bf2:/home/${USER}/${SWITCH_ROOTPATH}/netreach-voting-v3/tofino
