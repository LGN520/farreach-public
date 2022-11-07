if [ "x${is_common_included}" != "x1" ]; then
	source scripts/common.sh
fi

#set -x

# NOTE: before running this script
# (1) you need to finish loading phase by launching nocache switch/server + load_and_backup.sh
# (2) you need to finish keydump phase by keydump_and_sync.sh
# (3) you need to finish setup phase by prepare_server_rotation.sh + launching switch of corresponding method with setuping MAT rules

##### Part 0 #####
echo "[part 0] clean up server storages"
ssh ${USER}@${SERVER0} "rm -r /tmp/${DIRNAME}/*"
ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/*"

if [[ ${with_controller} -eq 1 ]]; then
	# NOTE: if w/ in-switch cache, finish warmup phase by launching servers of correpsonding method + warmup_client + stopping servers
	echo "[part 0] pre-admit hot keys into switch before server rotation"

	echo "launch storage servers of ${DIRNAME}"
	source scripts/remote/launchservertestbed.sh
	sleep 10s

	echo "pre-admit hot keys"
	cd ${DIRNAME}
	./warmup_client
	cd ..
	sleep 10s

	echo "stop storage servers of ${DIRNAME}"
	source scripts/remote/stopservertestbed.sh
fi

##### Part 1 #####

echo "[part 1] run single bottleneck server thread"

echo "clear tmp files in remote clients/servers and controller"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out; rm tmp_controller_bwcost.out"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part2*.out"

source scripts/remote/test_server_rotation_p1.sh 0

##### Part 2 #####

echo "[part 2] run bottleneck server thread + rotated server thread"

rotatecnt=1
for rotateidx in $(seq 0 $(expr ${server_total_logical_num_for_rotation} - 1)); do
	if [ ${rotateidx} -eq ${bottleneck_serveridx} ]; then
		continue
	fi

	#if [ ${rotatecnt} -ne 0 ] && [ $((${rotatecnt}%16)) -eq 0 ]; then
	if [ $((${rotatecnt}%8)) -eq 0 ]; then
		echo "refresh bottleneck parition and rotated partition back to the state after loading phase"
		ssh ${USER}@${SERVER0} "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker${bottleneck_serveridx}.db" # retrieve rocksdb and reset bottleneckserver/controller.snapshotid = 0
		ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker${rotateidx}.db" # retrieve rocksdb and reset rotatedservers.snapshotid = 0
	else
		ssh ${USER}@${SERVER1} "mv /tmp/${DIRNAME}/worker*.db /tmp/${DIRNAME}/worker${rotateidx}.db"
	fi

	source scripts/remote/test_server_rotation_p2.sh 0 ${rotateidx}
	rotatecnt=$((++rotatecnt))

	#read -p "Continue[y/n]: " is_continue
	#if [ ${is_continue}x == nx ]
	#then
	#	exit
	#fi
done

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
