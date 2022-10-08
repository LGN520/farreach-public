source ../scripts/common.sh
DIRNAME="nocache"

echo "stop servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"

echo "kill servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
