source ../scripts/common.sh
DIRNAME="distfarreach"

# NOTE: you need to launch spine/leaf switch data plane and local control plane before running this script

echo "clear tmp files in remote servers"
sudo rm tmp_reflector.out
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out; rm tmp_reflector.out; rm tmp_controller.out"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out"

bash remotescripts/stopservertestbed.sh

#echo "clear system cache"
#ssh -t ${USER}@dl16 "echo 123456 | sudo -S sh -c 'echo 3 > /proc/sys/vm/drop_caches'; echo 123456 | sudo -S sh -c 'echo 4 > /proc/sys/vm/drop_caches'"
#ssh -t ${USER}@dl13 "echo 123456 | sudo -S sh -c 'echo 3 > /proc/sys/vm/drop_caches'; echo 123456 | sudo -S sh -c 'echo 4 > /proc/sys/vm/drop_caches'"

echo "launch controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >tmp_controller.out 2>&1 &"

echo "launch servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >tmp_server.out 2>&1 &"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 1 >tmp_server.out 2>&1 &"

echo "launch reflectors"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./reflector leaf >tmp_reflector.out 2>&1 &"
sudo nohup ./reflector spine >tmp_reflector.out 2>&1 &
