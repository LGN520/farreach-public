set -x
if [ "x${is_common_included}" != "x1" ]; then
	source scriptsdist/common.sh
fi

#
set -e

if [ $# -ne 0 ]; then
	echo "Usage: bash scriptsdist/remote/test_server_rotation_latency.sh"
	exit
fi

# Get static throughput statistics under server rotation (workloadmode must be 0)

if [ "x${workloadmode}" == "x0" ]; then
	thpt_midstr="static${server_total_logical_num_for_rotation}"
else
	echo "Invalid workloadmode: ${workloadmode}"
	exit
fi

methodname=${DIRNAME}
statisticsdir="${workloadname}-statistics"
filedir=${CLIENT_ROOTPATH}/benchmark/output/${statisticsdir}

localthptfilename="${methodname}-${thpt_midstr}-client0.out"
remotethptfilename="${methodname}-${thpt_midstr}-client1.out"

localthptfilepath=${filedir}/${localthptfilename}
remotethptfilepath=${filedir}/${remotethptfilename}

echo "copy thpt statistics file ${remotethptfilename} from another client"
# scp ${USER}@${SECONDARY_CLIENT}:${remotethptfilepath} ${filedir}

cd scriptsdist/local/
# python calculate_target_helper.py ${localthptfilepath} ${remotethptfilepath} ${bottleneck_serveridx}

perrotation_targets=($(python calculate_target_helper.py ${localthptfilepath} ${remotethptfilepath} ${bottleneck_serveridx}))
echo "Per-rotation targets: ${perrotation_targets[*]}"
cd ../../

set +e

# TODO: invoke test_server_rotation_latency_<p0/p1/p2>.sh (similar as test_server_rotation.sh)
# NOTE: pass perrotation_targets[i] to the i-th rotation when starting clients

##### Part 0 #####
echo "[part 0] clean up server storages"

# ssh ${USER}@${SERVER0} "
rm -r /tmp/${DIRNAME}/*
# ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/*"

if [[ ${with_controller} -eq 1 ]]; then
	# NOTE: if w/ in-switch cache, finish warmup phase by launching servers of correpsonding method + warmup_client + stopping servers
	echo "[part 0] pre-admit hot keys into switch before server rotation"

	echo "launch storage servers of ${DIRNAME}"
	source scriptsdist/remote/launchservertestbed.sh
	sleep 10s

	echo "pre-admit hot keys"
	cd ${DIRNAME}
	mx h1 ./warmup_client
	cd ..
	sleep 120s

	echo "stop storage servers of ${DIRNAME}"
	source scriptsdist/remote/stopservertestbed.sh
fi

##### Part 1 #####
echo "[part 1] run single bottleneck server thread"

echo "clear tmp files in remote clients/servers and controller"
# ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out"
# ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out; rm tmp_controller_bwcost.out"
# ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part2*.out"

echo ${perrotation_targets[0]}

if [ "x${DIRNAME}" == "xfarreach" ]; then
	# clear snapshot token every iteration to maintain snapshot id sequence
	# ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "
	cd ${SWITCH_ROOTPATH}/${DIRNAME}/bmv2; bash cleanup_obselete_snapshottoken.sh >tmp_cleanup.out 2>&1
fi

source scriptsdist/remote/test_server_rotation_p1.sh 0 ${perrotation_targets[0]}

##### Part 2 #####
echo "[part 2] run bottleneck server thread + rotated server thread"
rotatecnt=1
for rotateidx in $(seq 0 $(expr ${server_total_logical_num_for_rotation} - 1)); do
	if [ ${rotateidx} -eq ${bottleneck_serveridx} ]; then
		continue
	fi

	if [ $((${rotatecnt}%8)) -eq 0 ]; then
		echo "refresh bottleneck parition and rotated partition back to the state after loading phase"
		# ssh ${USER}@${SERVER0} "
		rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker${bottleneck_serveridx}.db # retrieve rocksdb and reset bottleneckserver/controller.snapshotid = 0
		# ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/*;
		cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker${rotateidx}.db           # retrieve rocksdb and reset rotatedservers.snapshotid = 0
	else
		# ssh ${USER}@${SERVER1} "
		mv /tmp/${DIRNAME}/worker*.db /tmp/${DIRNAME}/worker${rotateidx}.db
	fi

	if [ "x${DIRNAME}" == "xfarreach" ]; then
		# clear snapshot token every iteration to maintain snapshot id sequence
		# ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "
		cd ${SWITCH_ROOTPATH}/${DIRNAME}/bmv2; bash cleanup_obselete_snapshottoken.sh >>tmp_cleanup.out 2>&1
	fi

	source scriptsdist/remote/test_server_rotation_p2.sh 0 ${rotateidx} ${perrotation_targets[${rotatecnt}]}
	rotatecnt=$((++rotatecnt))
done

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
