if [ "x${is_common_included}" != "x1" ]; then
	source scripts/common.sh
fi

#set -x

if [ $# -lt 2 ]; then
	echo "Usage: bash scripts/remote/test_server_rotation_p2.sh <isSingleRotation> <rotationidx> [targetthpt]"
	exit
fi

tmpsinglerotation=$1
if [ ${tmpsinglerotation} -ne 1 ] && [ ${tmpsinglerotation} -ne 0 ]; then
	echo "isSingleRotation should be 1 or 0"
	exit
fi

tmprotateidx=$2
echo "tmprotateidx: "${tmprotateidx}

tmptargetthpt=-1
if [ $# -eq 3 ]; then
	tmptargetthpt=$3
fi
tmptargetthpt=${tmptargetthpt%.*}

echo "stop clients"
source bash scripts/local/localstop.sh ycsb >/dev/null 2>&1
sleep 5s
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localstop.sh ycsb >/dev/null 2>&1"
echo "kill clients"
source scripts/local/localkill.sh ycsb >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localkill.sh ycsb >/dev/null 2>&1"
# stop and kill server/controller/reflector
source scripts/remote/stopservertestbed.sh

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
sed -i '1,$s/server_logical_idxes=TODO1/server_logical_idxes='${tmprotateidx}/'' ${DIRNAME}/config.ini
if [ "x${DIRNAME}" == "xfarreach" ]
then
	sed -i '1,$s/controller_snapshot_period=TODO/controller_snapshot_period='${snapshot_period}/'' ${DIRNAME}/config.ini
	sed -i '1,$s/switch_kv_bucket_num=TODO/switch_kv_bucket_num='${cache_size}/'' ${DIRNAME}/config.ini
fi
source scripts/remote/sync_file.sh ${DIRNAME} config.ini

echo "start servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >>tmp_serverrotation_part2_server.out 2>&1 &"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 1 >>tmp_serverrotation_part2_server.out 2>&1 &"
if [ ${with_controller} -eq 1 ]; then
	echo "start controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >>tmp_serverrotation_part2_controller.out 2>&1 &"
fi
if [ ${with_reflector} -eq 1 ]; then
	echo "start reflectors"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./reflector leaf >>tmp_serverrotation_part2_reflector.out 2>&1 &"
	cd ${DIRNAME}
	sudo nohup ./reflector spine >>tmp_serverrotation_part2_reflector.out 2>&1 &
	cd ..
fi

justloaded=0
if [ ${tmprotateidx} -lt ${bottleneck_serveridx} ]; then
	# 0, 1, 2, ..., 13 -> 1, 2, 3, ..., 14
	justloaded=$(expr ${tmprotateidx} + 1)
else
	# 15 -> 15
	justloaded=$(expr ${tmprotateidx})
fi

if [ $((${justloaded} % 8)) -eq 0 ]; then
	echo "sleep 120s after retrieving database files"
	sleep 120s
else
	echo "sleep 5s"
	sleep 5s
fi

echo "start clients"
if [ $# -eq 3 ]; then
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 -sr ${tmpsinglerotation} -target ${tmptargetthpt} >>tmp_serverrotation_part2_client.out 2>&1 &"
else
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 -sr ${tmpsinglerotation} >>tmp_serverrotation_part2_client.out 2>&1 &"
fi
sleep 1s
pwd
cd ${CLIENT_ROOTPATH}/benchmark/ycsb/
if [ $# -eq 3 ]; then
	./bin/ycsb run ${DIRNAME} -pi 0 -sr ${tmpsinglerotation} -target ${tmptargetthpt}
else
	./bin/ycsb run ${DIRNAME} -pi 0 -sr ${tmpsinglerotation}
fi
cd ../../

# stop and kill server/controller/reflector
source scripts/remote/stopservertestbed.sh
