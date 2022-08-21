DIRNAME="farreach"

echo "stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"

echo "stop controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"

#echo "stop reflector"
#ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_reflector.sh >/dev/null 2>&1"

echo "kill servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
