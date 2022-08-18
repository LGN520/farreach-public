DIRNAME="nocache"

echo "stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"

echo "kill servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
