#!/use/bin/env bash

echo "kill test_server_rotation.sh"
clientpids=( "$(ps -aux | grep "./test_server_rotation" | grep -v "grep" | awk '{print $2}')" )

if [ ${#clientpids[@]} -gt 0 ]
then
	for i in ${!clientpids[@]}
	do
		if [ "${clientpids[i]}x" != "x" ]
		then
			kill -9 ${clientpids[i]}
		fi
	done
fi

echo "stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
echo "stop clients"
bash localscripts/stop_client.sh >/dev/null 2>&1
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_client.sh >/dev/null 2>&1"
echo "stop controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"

echo "stop reflectors"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_reflector.sh >/dev/null 2>&1"
sudo bash localscripts/stop_reflector.sh

echo "kill servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
echo "kill clients"
bash localscripts/kill_client.sh >/dev/null 2>&1
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_client.sh >/dev/null 2>&1"
echo "kill controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_controller.sh >/dev/null 2>&1"
