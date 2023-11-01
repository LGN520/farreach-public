set -x
if [ "x${is_common_included}" != "x1" ]; then
	source scriptsbmv2/common.sh
fi

#

# NOTE: before running this script
# (1) you need to finish loading phase by launching nocache switch/server + load_and_backup.sh
# (2) you need to finish keydump phase by keydump_and_sync.sh
# (3) you need to finish setup phase by prepare_server_rotation.sh + launching switch of corresponding method with setuping MAT rules

##### Part 0 #####
echo "[part 0] clean up server storages"
rm -r /tmp/${DIRNAME}/*

if [[ ${with_controller} -eq 1 ]]; then
	# NOTE: if w/ in-switch cache, finish warmup phase by launching servers of correpsonding method + warmup_client + stopping servers
	echo "[part 0] pre-admit hot keys into switch before server rotation"

	echo "launch storage servers of ${DIRNAME}"
	source scriptsbmv2/remote/launchservertestbed.sh
	#sleep 10s
	sleep 10s

	echo "pre-admit hot keys"
	cd ${DIRNAME}
	mx h1 ./warmup_client
	cd ..
	sleep 120s

	echo "stop storage servers of ${DIRNAME}"
	source scriptsbmv2/remote/stopservertestbed.sh
fi

##### Part 1 #####
echo "[part 1] run single bottleneck server thread"

echo "clear tmp files in remote clients/servers and controller"

if [ "x${DIRNAME}" == "xfarreach" ]; then
	# clear snapshot token every iteration to maintain snapshot id sequence
	# ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "
	cd ${SWITCH_ROOTPATH}/${DIRNAME}/bmv2; 
	mx switchos bash cleanup_obselete_snapshottoken.sh >tmp_cleanup.out 2>&1
fi
cd ${SWITCH_ROOTPATH}

# Retrieve both dl16.bottleneckserver and dl13.rotatedservers to the state just after loading phase
echo "retrieve both bottleneck partition and rotated partition back to the state after loading phase"
# ssh ${USER}@${SERVER0} "
rm -r /tmp/${DIRNAME}/*; 
cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker${bottleneck_serveridx}.db # retrieve rocksdb and reset bottleneckserver/controller.snapshotid = 0
cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker0.db                       # retrieve rocksdb and reset rotatedservers.snapshotid = 0
# run twice for some sync bug
source scriptsbmv2/remote/test_server_rotation_p1.sh 0
source scriptsbmv2/remote/test_server_rotation_p1.sh 0

##### Part 2 #####
echo "[part 2] run bottleneck server thread + rotated server thread"

rotatecnt=1
for rotateidx in $(seq 0 $(expr ${server_total_logical_num_for_rotation} - 1)); do
	if [ ${rotateidx} -eq ${bottleneck_serveridx} ]; then
		continue
	fi

	# initiliaze and refresh database status every 8 iterations
	# no need for bmv2

	if [ "x${DIRNAME}" == "xfarreach" ]; then
		cd ${SWITCH_ROOTPATH}/${DIRNAME}/bmv2; bash cleanup_obselete_snapshottoken.sh >>tmp_cleanup.out 2>&1
	fi
	cd ${SWITCH_ROOTPATH}
	source scriptsbmv2/remote/test_server_rotation_p2.sh 0 ${rotateidx}
	rotatecnt=$((++rotatecnt))
done

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
