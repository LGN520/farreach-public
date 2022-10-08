source ../scripts/common.sh
DIRNAME="distnocache"

# NOTE: you need to launch spine/leaf switch data plane and local control plane before running this script

echo "clear tmp files in remote servers"
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; rm tmp_server.out"
ssh ${USER}@dl13 "cd projects/NetBuffer/${DIRNAME}; rm tmp_server.out"

bash remotescripts/stopservertestbed.sh

echo "launch servers"
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >tmp_server.out 2>&1 &"
ssh ${USER}@dl13 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 1 >tmp_server.out 2>&1 &"
