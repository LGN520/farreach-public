source ../scripts/common.sh
DIRNAME="distnocache"

echo "stop servers"
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ${USER}@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"

echo "kill servers"
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ${USER}@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
