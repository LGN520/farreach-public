DIRNAME="distnocache"

# NOTE: you need to finish loading phase [+ warmup phase (if w/ inswitch cache)] before running this script

#if [ $# -ne 1 ]
#then
#	echo "Usage: bash test_server_rotation.sh bottleneck_serveridx"
#	exit
#fi

#bottleneck_serveridx=$1
bottleneck_serveridx=123
server_total_logical_num_for_rotation=128

echo "clear tmp files in remote clients/servers"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; rm tmp0.out; rm tmp.out"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; rm tmp0.out; rm tmp.out"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; rm tmp.out"

echo "[part 1] run single bottleneck server thread"

echo "stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
echo "stop clients"
bash localscripts/stop_client.sh >/dev/null 2>&1
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_client.sh >/dev/null 2>&1"
echo "kill servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
echo "kill clients"
bash localscripts/kill_client.sh >/dev/null 2>&1
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_client.sh >/dev/null 2>&1"
sleep 1s

# TODO: retrieve dl16.bottleneckserver to the state just after loading phase
echo "retrieve bottleneck partition back to the state after loading phase"
#ssh ssy@dl16 "rm -r /tmp/${DIRNAME}/*"
#ssh ssy@dl13 "rm -r /tmp/${DIRNAME}/*"

echo "prepare and sync config.ini"
cp configs/config.ini.rotation-transaction1p.dl16dl13 config.tmp
sed -e '87s/server_logical_idxes=123/server_logical_idxes='${bottleneck_serveridx}'/g' config.tmp > config.ini
rm config.tmp
bash sync_file.sh config.ini

echo "start servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >tmp0.out 2>&1 &"
sleep 5s

echo "start clients"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; nohup ./remote_client 1 >tmp0.out 2>&1 &"
sleep 10s
./remote_client 0

echo "stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
sleep 5s
echo "kill servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"






echo "[part 2] run bottleneck server thread + rotated server thread"

for rotateidx in $(seq 0 $(expr ${server_total_logical_num_for_rotation} - 1))
do
	if [ ${rotateidx} -eq ${bottleneck_serveridx} ]
	then
		continue
	fi

	echo "rotateidx: "${rotateidx}

	echo "stop servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null"
	echo "stop clients"
	bash localscripts/stop_client.sh >/dev/null 2>&1
	ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_client.sh >/dev/null"
	sleep 1s
	echo "kill servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
	echo "kill clients"
	bash localscripts/kill_client.sh >/dev/null 2>&1
	ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_client.sh >/dev/null 2>&1"

	# TODO: retrieve dl16.bottleneckserver to the state just after loading phase
	echo "retrieve bottleneck partition back to the state after loading phase"
	#ssh ssy@dl16 "rm -r /tmp/${DIRNAME}/*"
	#ssh ssy@dl13 "rm -r /tmp/${DIRNAME}/*"

	echo "prepare and sync config.ini"
	cp configs/config.ini.rotation-transaction2p.dl16dl13 config.tmp
	sed -e '87s/server_logical_idxes=123/server_logical_idxes='${bottleneck_serveridx}'/g' -e '107s/server_logical_idxes=0/server_logical_idxes='${rotateidx}'/g' config.tmp > config.ini
	rm config.tmp
	bash sync_file.sh config.ini

	echo "start servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >>tmp.out 2>&1 &"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 1 >>tmp.out 2>&1 &"
	sleep 5s

	echo "start clients"
	ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; nohup ./remote_client 1 >>tmp.out 2>&1 &"
	sleep 10s
	./remote_client 0

	echo "stop servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null"
	sleep 5s
	echo "kill servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"

	#read -p "Continue[y/n]: " is_continue
	#if [ ${is_continue}x == nx ]
	#then
	#	exit
	#fi
done
