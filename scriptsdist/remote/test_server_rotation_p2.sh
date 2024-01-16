if [ "x${is_common_included}" != "x1" ]; then
	source scriptsdist/common.sh
fi

#

if [ $# -lt 2 ]; then
	echo "Usage: bash scriptsdist/remote/test_server_rotation_p2.sh <isSingleRotation> <rotationidx> [targetthpt]"
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
source bash scriptsdist/local/localstop.sh ycsb >/dev/null 2>&1
sleep 5s
# ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scriptsdist/local/localstop.sh ycsb >/dev/null 2>&1"
echo "kill clients"
source scriptsdist/local/localkill.sh ycsb >/dev/null 2>&1
# ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scriptsdist/local/localkill.sh ycsb >/dev/null 2>&1"
# stop and kill server/controller/reflector
source scriptsdist/remote/stopservertestbed.sh

# TODO: only retrieve dl16.bottleneckserver to the state just after loading phase
# NOTE: do NOT overwrite rocksdb files to avoid long time of loading overwritten files, which does NOT affect average performance
echo "retrieve bottleneck partition back to the state after loading phase"
rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot* # retrieve bottleneckserver/controller.snapshotid = 0
# ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/worker*snapshot*; rm -r /tmp/${DIRNAME}/controller*snapshot*" # retrieve bottleneckserver.snapshotid = 0
cd ${SWITCH_ROOTPATH}
echo "prepare and sync config.ini"
cp ${DIRNAME}/configs/config.ini.static.2p.bmv2 ${DIRNAME}/config.ini
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
# source scriptsdist/remote/sync_file.sh ${DIRNAME} config.ini
cd ${SWITCH_ROOTPATH}/${DIRNAME}
echo "start servers"
mx h3  ./server 0 >tmp_serverrotation_part2_server0.out 2>&1 &
sleep 30s
mx h4  ./server 1 >tmp_serverrotation_part2_server1.out 2>&1 &
sleep 30s

# sleep 30s
# sleep 30s
# sleep 30s
if [ ${with_controller} -eq 1 ]; then
	echo "start controller"
	mx h3  ./controller >>tmp_serverrotation_part2_controller.out 2>&1 &
	sleep 10s
fi
if [ ${with_reflector} -eq 1 ]; then
	echo "start reflectors"
	mx h3  ./reflector leaf >>tmp_serverrotation_part2_reflector.out 2>&1 &
	cd ${DIRNAME}
	mx h3 sudo  ./reflector spine >>tmp_serverrotation_part2_reflector.out 2>&1 &
	cd ..
	sleep 10s
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
	# if [ "x${DIRNAME}" == "xfarreach" ]
	# then
	# 	# sleep 120s
	# fi
	sleep 120s # wait longer time for rotations performing database file retrieving
else
	echo "sleep 5s"
	sleep 5s
fi

echo "start clients"
if [ $# -eq 3 ]; then
	# ssh ${USER}@${SECONDARY_CLIENT} "
	cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; 
	mx h2  python2 ./bin/ycsb run ${DIRNAME} -pi 1 -sr ${tmpsinglerotation} -target ${tmptargetthpt} >>tmp_serverrotation_part2_client1.out 2>&1 &
else
	# ssh ${USER}@${SECONDARY_CLIENT} "
	cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; 
	mx h2  python2 ./bin/ycsb run ${DIRNAME} -pi 1 -sr ${tmpsinglerotation} >>tmp_serverrotation_part2_client1.out 2>&1 &
fi
sleep 20s
pwd
cd ${CLIENT_ROOTPATH}/benchmark/ycsb/
if [ $# -eq 3 ]; then
	mx h1 python2 ./bin/ycsb run ${DIRNAME} -pi 0 -sr ${tmpsinglerotation} -target ${tmptargetthpt}
else
	mx h1 python2 ./bin/ycsb run ${DIRNAME} -pi 0 -sr ${tmpsinglerotation}
fi
cd ../../

# stop and kill server/controller/reflector
source scriptsdist/remote/stopservertestbed.sh
