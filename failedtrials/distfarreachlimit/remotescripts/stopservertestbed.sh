source ../scripts/common.sh
DIRNAME="distfarreachlimit"

echo "stop controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"

echo "stop servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"

echo "stop reflectors"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/stop_reflector.sh >/dev/null 2>&1"
sudo bash localscripts/stop_reflector.sh
