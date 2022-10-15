if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

with_controller=0
if [ "x${DIRNAME}" == "xfarreach" ] || [ "x${DIRNAME}" == "xnetcache" ] || [ "x${DIRNAME}" == "xdistfarreach" ] || [ "x${DIRNAME}" == "xdistcache" ]
then
	with_controller=1
fi

# NOTE: you need to finish loading phase + setup phase (including warmup phase if w/ inswitch cache) before running this script

# NOTE: we trigger snapshot in the physical client 0 during transaction phase for farreach/distfarreach

echo "clear tmp files in remote clients/servers and controller"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}/benchmark/ycsb/; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out; rm tmp_controller_bwcost.out"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part2*.out"

echo "[part 1] run single bottleneck server thread"

echo "stop servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
echo "stop clients"
source bash scripts/local/localstop.sh ./client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localstop.sh ./client >/dev/null 2>&1"
if [ ${with_controller} -eq 1 ]
then
	echo "stop controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./controller >/dev/null 2>&1"
fi
#sleep 1s
echo "kill servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
echo "kill clients"
source scripts/local/localkill.sh ./client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localkill.sh ./client >/dev/null 2>&1"
if [ ${with_controller} -eq 1 ]
then
	echo "kill controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./controller >/dev/null 2>&1"
fi

# Retrieve both dl16.bottleneckserver and dl13.rotatedservers to the state just after loading phase
echo "retrieve both bottleneck partition and rotated partitions back to the state after loading phase"
ssh ${USER}@${SERVER0} "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/${server_total_logical_num_for_rotation}/worker${bottleneck_serveridx}.* /tmp/${DIRNAME}/" # retrieve bottleneckserver rocksdb and reset bottleneckserver/controller.snapshotid = 0
ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/${server_total_logical_num_for_rotation}/* /tmp/${DIRNAME}/" # retrieve rotatedservers' rocksdb and reset rotatedservers.snapshotid = 0

echo "prepare and sync config.ini"
cp ${DIRNAME}/configs/config.ini.static.1p ${DIRNAME}/config.ini
sed -i '1,$s/workload_name=TODO/workload_name='${workloadname}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_total_logical_num_for_rotation=TODO/server_total_logical_num_for_rotation='${server_total_logical_num_for_rotation}/'' ${DIRNAME}/config.ini
sed -i '1,$s/bottleneck_serveridx_for_rotation=TODO/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_logical_idxes=TODO0/server_logical_idxes='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
source scripts/remote/sync_file.sh ${DIRNAME} config.ini

echo "start servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >tmp_serverrotation_part1_server.out 2>&1 &"
if [ ${with_controller} -eq 1 ]
then
	echo "start controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >tmp_serverrotation_part1_controller.out 2>&1 &"
fi
sleep 15s # wait longer time for the first rotation, as rocksdb needs to load the files overwritten by the backups

echo "start clients"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 >tmp_serverrotation_part1_client.out 2>&1 &"
sleep 5s
cd benchmark/ycsb/
./bin/ycsb run ${DIRNAME} -pi 0
cd ../../

echo "stop servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
if [ ${with_controller} -eq 1 ]
then
	echo "stop controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./controller >/dev/null 2>&1"
fi
sleep 5s
echo "kill servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
if [ ${with_controller} -eq 1 ]
then
	echo "kill controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./controller >/dev/null 2>&1"
fi






echo "[part 2] run bottleneck server thread + rotated server thread"

for rotateidx in $(seq 0 $(expr ${server_total_logical_num_for_rotation} - 1))
do
	if [ ${rotateidx} -eq ${bottleneck_serveridx} ]
	then
		continue
	fi

	echo "rotateidx: "${rotateidx}

	echo "stop servers"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
	echo "stop clients"
	source bash scripts/local/localstop.sh ./client >/dev/null 2>&1
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localstop.sh ./client >/dev/null 2>&1"
	if [ ${with_controller} -eq 1 ]
	then
		echo "stop controller"
		ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./controller >/dev/null 2>&1"
	fi
	#sleep 1s
	echo "kill servers"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
	echo "kill clients"
	source scripts/local/localkill.sh ./client >/dev/null 2>&1
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localkill.sh ./client >/dev/null 2>&1"
	if [ ${with_controller} -eq 1 ]
	then
		echo "kill controller"
		ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./controller >/dev/null 2>&1"
	fi

	# TODO: only retrieve dl16.bottleneckserver to the state just after loading phase
	# NOTE: do NOT overwrite rocksdb files to avoid long time of loading overwritten files, which does NOT affect average performance
	echo "retrieve bottleneck partition back to the state after loading phase"
	ssh ${USER}@${SERVER0} "rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot*" # retrieve bottleneckserver/controller.snapshotid = 0
	ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot*" # retrieve bottleneckserver.snapshotid = 0

	echo "prepare and sync config.ini"
	cp ${DIRNAME}/configs/config.ini.static.2p ${DIRNAME}/config.ini
	sed -i '1,$s/workload_name=TODO/workload_name='${workloadname}/'' ${DIRNAME}/config.ini
	sed -i '1,$s/server_total_logical_num_for_rotation=TODO/server_total_logical_num_for_rotation='${server_total_logical_num_for_rotation}/'' ${DIRNAME}/config.ini
	sed -i '1,$s/bottleneck_serveridx_for_rotation=TODO/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
	sed -i '1,$s/server_logical_idxes=TODO0/server_logical_idxes='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
	sed -i '1,$s/server_logical_idxes=TODO1/server_logical_idxes='${rotateidx}/'' ${DIRNAME}/config.ini
	source scripts/remote/sync_file.sh ${DIRNAME} config.ini

	echo "start servers"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >tmp_serverrotation_part1_server.out 2>&1 &"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 1 >>tmp_serverrotation_part2_server.out 2>&1 &"
	if [ ${with_controller} -eq 1 ]
	then
		echo "start controller"
		ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >tmp_serverrotation_part1_controller.out 2>&1 &"
	fi
	sleep 5s

	echo "start clients"
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 >tmp_serverrotation_part1_client.out 2>&1 &"
	sleep 5s
	cd benchmark/ycsb/
	./bin/ycsb run ${DIRNAME} -pi 0
	cd ../../

	echo "stop servers"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
	if [ ${with_controller} -eq 1 ]
	then
		echo "stop controller"
		ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./controller >/dev/null 2>&1"
	fi
	sleep 5s
	echo "kill servers"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
	if [ ${with_controller} -eq 1 ]
	then
		echo "kill controller"
		ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./controller >/dev/null 2>&1"
	fi

	#read -p "Continue[y/n]: " is_continue
	#if [ ${is_continue}x == nx ]
	#then
	#	exit
	#fi
done

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
