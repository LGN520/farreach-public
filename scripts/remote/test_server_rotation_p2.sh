if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

if [ $# -ne 2 ]
then
	echo "Usage: bash scripts/remote/test_server_rotation_p2.sh <isSingleRotation> <rotationidx>"
	exit
fi

tmpsinglerotation=$1
if [ ${tmpsinglerotation} -ne 1 ] && [ ${tmpsinglerotation} -ne 0 ]
then
	echo "isSingleRotation should be 1 or 0"
	exit
fi

tmprotateidx=$2
echo "tmprotateidx: "${tmprotateidx}

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
source scripts/remote/sync_file.sh ${DIRNAME} config.ini

echo "start servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >>tmp_serverrotation_part2_server.out 2>&1 &"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 1 >>tmp_serverrotation_part2_server.out 2>&1 &"
if [ ${with_controller} -eq 1 ]
then
	echo "start controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >>tmp_serverrotation_part2_controller.out 2>&1 &"
fi
if [ ${with_reflector} -eq 1 ]
then
	echo "start reflectors"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./reflector leaf >>tmp_serverrotation_part2_reflector.out 2>&1 &"
	cd ${DIRNAME}
	sudo nohup ./reflector spine >>tmp_serverrotation_part2_reflector.out 2>&1 &
	cd ..
fi


justloaded=0
if [ ${tmprotateidx} -lt ${bottleneck_serveridx} ]; then
	justloaded=${tmprotateidx}
else
	justloaded=$((${tmprotateidx}-1))
fi

if [ $((${justloaded}%16)) -eq 0 ]; then
	echo "sleep 120s"
	sleep 120s
else
	echo "sleep 5s"
	sleep 5s
fi

echo "start clients"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 -sr ${tmpsinglerotation} >>tmp_serverrotation_part2_client.out 2>&1 &"
sleep 1s
pwd
cd ${CLIENT_ROOTPATH}/benchmark/ycsb/
./bin/ycsb run ${DIRNAME} -pi 0
cd ../../

# stop and kill server/controller/reflector
source scripts/remote/stopservertestbed.sh
