source ../../scripts/common.sh
ssh ssy@bf2 "rm -rf /home/${USER}/${CLIENT_ROOTPATH}/netreach-v4/tofino"
scp -r ../tofino ssy@bf2:/home/${USER}/${CLIENT_ROOTPATH}/netreach-v4/tofino
