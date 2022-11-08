if [ "x${is_common_included}" != "x1" ]; then
	source scripts/common.sh
fi

#set -x

if [ $# -lt 1 ]; then
	echo "Usage: bash scripts/remote/test_server_rotation_p1.sh <isSingleRotation> [targetthpt]"
	exit
fi

tmpsinglerotation=$1
if [ ${tmpsinglerotation} -ne 1 ] && [ ${tmpsinglerotation} -ne 0 ]; then
	echo "isSingleRotation should be 1 or 0"
	exit
fi

tmptargetthpt=-1
if [ $# -eq 2 ]; then
	tmptargetthpt=$2
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

# Retrieve both dl16.bottleneckserver and dl13.rotatedservers to the state just after loading phase
echo "retrieve both bottleneck partition and rotated partition back to the state after loading phase"
ssh ${USER}@${SERVER0} "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker${bottleneck_serveridx}.db" # retrieve rocksdb and reset bottleneckserver/controller.snapshotid = 0
ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/*; cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker0.db"                       # retrieve rocksdb and reset rotatedservers.snapshotid = 0

echo "prepare and sync config.ini"
cp ${DIRNAME}/configs/config.ini.static.1p ${DIRNAME}/config.ini
sed -i '1,$s/workload_name=TODO/workload_name='${workloadname}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_total_logical_num_for_rotation=TODO/server_total_logical_num_for_rotation='${server_total_logical_num_for_rotation}/'' ${DIRNAME}/config.ini
sed -i '1,$s/bottleneck_serveridx_for_rotation=TODO/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_logical_idxes=TODO0/server_logical_idxes='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
if [ "x${DIRNAME}" == "xfarreach" ]
then
	sed -i '1,$s/controller_snapshot_period=TODO/controller_snapshot_period='${snapshot_period}/'' ${DIRNAME}/config.ini
	sed -i '1,$s/switch_kv_bucket_num=TODO/switch_kv_bucket_num='${cache_size}/'' ${DIRNAME}/config.ini
fi
source scripts/remote/sync_file.sh ${DIRNAME} config.ini

echo "start servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >tmp_serverrotation_part1_server.out 2>&1 &"
if [ ${with_controller} -eq 1 ]; then
	echo "start controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >tmp_serverrotation_part1_controller.out 2>&1 &"
fi
if [ ${with_reflector} -eq 1 ]; then
	echo "start reflectors"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./reflector leaf >tmp_serverrotation_part1_reflector.out 2>&1 &"
	cd ${DIRNAME}
	sudo nohup ./reflector spine >tmp_serverrotation_part1_reflector.out 2>&1 &
	cd ..
fi
sleep 120s # wait longer time for the first rotation, as rocksdb needs to load the files overwritten by the backups

# NOTE: we trigger snapshot in the physical client 0 during transaction phase for farreach/distfarreach
echo "start clients"
if [ $# -eq 2 ]; then
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 -sr ${tmpsinglerotation} -target ${tmptargetthpt} >tmp_serverrotation_part1_client.out 2>&1 &"
else
	ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 -sr ${tmpsinglerotation} >tmp_serverrotation_part1_client.out 2>&1 &"
fi
sleep 1s

cd ${CLIENT_ROOTPATH}/benchmark/ycsb/
if [ $# -eq 2 ]; then
	./bin/ycsb run ${DIRNAME} -pi 0 -sr ${tmpsinglerotation} -target ${tmptargetthpt}
else
	./bin/ycsb run ${DIRNAME} -pi 0 -sr ${tmpsinglerotation}
fi
cd ../../

# stop and kill server/controller/reflector
source scripts/remote/stopservertestbed.sh
