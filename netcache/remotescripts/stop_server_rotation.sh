source ../scripts/common.sh
#!/use/bin/env bash

DIRNAME="netcache"

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
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ${USER}@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
echo "stop clients"
bash localscripts/stop_client.sh >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_client.sh >/dev/null 2>&1"
echo "stop controller"
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"

echo "kill servers"
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ${USER}@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
echo "kill clients"
bash localscripts/kill_client.sh >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_client.sh >/dev/null 2>&1"
echo "kill controller"
ssh ${USER}@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_controller.sh >/dev/null 2>&1"
