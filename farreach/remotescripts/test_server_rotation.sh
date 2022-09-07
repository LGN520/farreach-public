DIRNAME="farreach"

# NOTE: you need to finish loading phase [+ warmup phase (if w/ inswitch cache)] before running this script

#if [ $# -ne 1 ]
#then
#	echo "Usage: bash test_server_rotation.sh bottleneck_serveridx"
#	exit
#fi

#bottleneck_serveridx=$1
bottleneck_serveridx=123
server_total_logical_num_for_rotation=128

# Change corresponding lines in configs/config.transaction1p or 2p
configfile_line1=27
configfile_line2=87
configfile_line3=107

# NOTE: we can trigger snapshot in client0 during transaction phase
#client_ready_time=13 # wait X seconds to trigger snapshot

echo "clear tmp files in remote clients/servers"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; rm tmp_serverrotation_part2*.out"

echo "[part 1] run single bottleneck server thread"

echo "stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
echo "stop clients"
bash localscripts/stop_client.sh >/dev/null 2>&1
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_client.sh >/dev/null 2>&1"
echo "stop controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"
echo "kill servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
echo "kill clients"
bash localscripts/kill_client.sh >/dev/null 2>&1
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_client.sh >/dev/null 2>&1"
echo "kill controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_controller.sh >/dev/null 2>&1"
sleep 1s

# TODO: retrieve dl16.bottleneckserver to the state just after loading phase
echo "retrieve bottleneck partition back to the state after loading phase"
ssh ssy@dl16 "rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot*" # retrieve bottleneckserver/controller.snapshotid = 0
#ssh ssy@dl13 "rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot*"
#ssh ssy@dl16 "rm -r /tmp/${DIRNAME}/*"
#ssh ssy@dl13 "rm -r /tmp/${DIRNAME}/*"

echo "prepare and sync config.ini"
cp configs/config.ini.rotation-transaction1p.dl16dl13 config.tmp
sed -e ''${configfile_line1}'s/bottleneck_serveridx_for_rotation=123/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}'/g' -e ''${configfile_line2}'s/server_logical_idxes=123/server_logical_idxes='${bottleneck_serveridx}'/g' config.tmp > config.ini
rm config.tmp
bash sync_file.sh config.ini

echo "start servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >tmp_serverrotation_part1_server.out 2>&1 &"
echo "start controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./controller >tmp_serverrotation_part1_controller.out 2>&1 &"
sleep 5s

echo "start clients"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; nohup ./remote_client 1 >tmp_serverrotation_part1_client.out 2>&1 &"
sleep 10s
#sleep ${client_ready_time}s && ./preparefinish_client & # wait X seconds after launching client 0, trigger controller.snapshot to check its influence -> set a proper X based on your testbed such that controller will make a snapshot during transaction phase to confirm the limited influence
./remote_client 0

echo "stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null 2>&1"
echo "stop controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"
sleep 5s
echo "kill servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
echo "kill controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_controller.sh >/dev/null 2>&1"






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
	echo "stop controller"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"
	sleep 1s
	echo "kill servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
	echo "kill clients"
	bash localscripts/kill_client.sh >/dev/null 2>&1
	ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_client.sh >/dev/null 2>&1"
	echo "kill controller"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_controller.sh >/dev/null 2>&1"

	# TODO: retrieve dl16.bottleneckserver to the state just after loading phase
	echo "retrieve bottleneck partition back to the state after loading phase"
	ssh ssy@dl16 "rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot*" # retrieve bottleneckserver/controller.snapshotid = 0
	#ssh ssy@dl13 "rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot*"
	#ssh ssy@dl16 "rm -r /tmp/${DIRNAME}/*"
	#ssh ssy@dl13 "rm -r /tmp/${DIRNAME}/*"

	echo "prepare and sync config.ini"
	cp configs/config.ini.rotation-transaction2p.dl16dl13 config.tmp
	sed -e ''${configfile_line1}'s/bottleneck_serveridx_for_rotation=123/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}'/g' -e ''${configfile_line2}'s/server_logical_idxes=123/server_logical_idxes='${bottleneck_serveridx}'/g' -e ''${configfile_line3}'s/server_logical_idxes=0/server_logical_idxes='${rotateidx}'/g' config.tmp > config.ini
	rm config.tmp
	bash sync_file.sh config.ini

	echo "start servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >>tmp_serverrotation_part2_server.out 2>&1 &"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 1 >>tmp_serverrotation_part2_server.out 2>&1 &"
	echo "start controller"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./controller >>tmp_serverrotation_part2_controller.out 2>&1 &"
	sleep 5s

	echo "start clients"
	ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; nohup ./remote_client 1 >>tmp_serverrotation_part2_client.out 2>&1 &"
	sleep 10s
	#sleep ${client_ready_time}s && ./preparefinish_client & # wait X seconds after launching client 0, trigger controller.snapshot to check its influence -> set a proper X based on your testbed such that controller will make a snapshot during transaction phase to confirm the limited influence
	./remote_client 0

	echo "stop servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_server.sh >/dev/null"
	echo "stop controller"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/stop_controller.sh >/dev/null 2>&1"
	sleep 5s
	echo "kill servers"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
	ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_server.sh >/dev/null 2>&1"
	echo "kill controller"
	ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash localscripts/kill_controller.sh >/dev/null 2>&1"

	#read -p "Continue[y/n]: " is_continue
	#if [ ${is_continue}x == nx ]
	#then
	#	exit
	#fi
done
