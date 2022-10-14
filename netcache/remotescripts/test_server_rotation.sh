source ../scripts/common.sh
DIRNAME="netcache"

set -x

# NOTE: you need to finish loading phase [+ warmup phase (if w/ inswitch cache)] before running this script

#if [ $# -ne 1 ]
#then
#	echo "Usage: bash test_server_rotation.sh bottleneck_serveridx"
#	exit
#fi

# Change corresponding lines in configs/config.transaction1p or 2p
configfile_line0=25
configfile_line1=27
configfile_line2=88
configfile_line3=108
echo "clear tmp files in remote clients/servers"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part1*.out; rm tmp_serverrotation_part2*.out"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_serverrotation_part2*.out"

echo "[part 1] run single bottleneck server thread"

echo "stop servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
echo "stop clients"
bash localscripts/localstop.sh client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh client >/dev/null"
echo "stop controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh controller >/dev/null"
echo "kill servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
echo "kill clients"
bash localscripts/localkill.sh client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh client >/dev/null"
echo "kill controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh controller >/dev/null"
sleep 1s

# TODO: retrieve both dl16.bottleneckserver and dl13.rotatedservers to the state just after loading phase
echo "retrieve both bottleneck partition and rotated partitions back to the state after loading phase"
ssh ${USER}@dl16 "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/${server_total_logical_num_for_rotation}/worker${bottleneck_serveridx}* /tmp/${DIRNAME}/" # retrieve bottleneckserver rocksdb and reset bottleneckserver/controller.snapshotid = 0
ssh ${USER}@dl13 "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/${server_total_logical_num_for_rotation}/* /tmp/${DIRNAME}/" # retrieve rotatedservers' rocksdb and reset rotatedservers.snapshotid = 0

echo "prepare and sync config.ini"
cp configs/config.ini.rotation-transaction1p.dl16dl13 config.tmp
sed -e ''${configfile_line0}'s/server_total_logical_num_for_rotation=128/server_total_logical_num_for_rotation='${server_total_logical_num}'/g' -e ''${configfile_line1}'s/bottleneck_serveridx_for_rotation=123/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}'/g' -e ''${configfile_line2}'s/server_logical_idxes=123/server_logical_idxes='${bottleneck_serveridx}'/g' config.tmp > config.ini
rm config.tmp
bash sync_file.sh config.ini

echo "start servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >tmp_serverrotation_part1_server.out 2>&1 &"
echo "start controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >tmp_serverrotation_part1_controller.out 2>&1 &"
sleep 10s

echo "start clients"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 >tmp_serverrotation_part1_client.out 2>&1 &"
sleep 10s
cd ${CLIENT_ROOTPATH}/benchmark/ycsb/
./bin/ycsb run ${DIRNAME} -pi 0
cd ${CLIENT_ROOTPATH}/${DIRNAME}

echo "stop servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
echo "stop controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh controller >/dev/null"
sleep 5s
echo "kill servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
echo "kill controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh controller >/dev/null"






echo "[part 2] run bottleneck server thread + rotated server thread"

for rotateidx in $(seq 0 $(expr ${server_total_logical_num_for_rotation} - 1))
do
	if [ ${rotateidx} -eq ${bottleneck_serveridx} ]
	then
		continue
	fi

	echo "rotateidx: "${rotateidx}

	echo "stop servers"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
	ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
	echo "stop clients"
	bash localscripts/localstop.sh client >/dev/null 2>&1
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh client >/dev/null"
	echo "stop controller"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh controller >/dev/null"
	sleep 1s
	echo "kill servers"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
	ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
	echo "kill clients"
	bash localscripts/localkill.sh client >/dev/null 2>&1
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh client >/dev/null"
	echo "kill controller"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh controller >/dev/null"

	# TODO: only retrieve dl16.bottleneckserver to the state just after loading phase
	# NOTE: do NOT overwrite rocksdb files to avoid long time of loading overwritten files, which does NOT affect average performance
	echo "retrieve bottleneck partition back to the state after loading phase"

	echo "prepare and sync config.ini"
	cp configs/config.ini.rotation-transaction2p.dl16dl13 config.tmp
	sed -e ''${configfile_line0}'s/server_total_logical_num_for_rotation=128/server_total_logical_num_for_rotation='${server_total_logical_num_for_rotation}'/g' -e ''${configfile_line1}'s/bottleneck_serveridx_for_rotation=123/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}'/g' -e ''${configfile_line2}'s/server_logical_idxes=123/server_logical_idxes='${bottleneck_serveridx}'/g' -e ''${configfile_line3}'s/server_logical_idxes=0/server_logical_idxes='${rotateidx}'/g' config.tmp > config.ini
	rm config.tmp
	bash sync_file.sh config.ini

	echo "start servers"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >>tmp_serverrotation_part2_server.out 2>&1 &"
	ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 1 >>tmp_serverrotation_part2_server.out 2>&1 &"
	echo "start controller"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >>tmp_serverrotation_part2_controller.out 2>&1 &"
	sleep 10s

	echo "start clients"
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 >>tmp_serverrotation_part2_client.out 2>&1 &"
	sleep 10s
	cd ${CLIENT_ROOTPATH}/benchmark/ycsb/
	./bin/ycsb run ${DIRNAME} -pi 0
	cd ${CLIENT_ROOTPATH}/${DIRNAME}

	echo "stop servers"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
	ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null"
	echo "stop controller"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh controller >/dev/null"
	sleep 5s
	echo "kill servers"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
	ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null"
	echo "kill controller"
	ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh controller >/dev/null"

	#read -p "Continue[y/n]: " is_continue
	#if [ ${is_continue}x == nx ]
	#then
	#	exit
	#fi
done
