if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x
#set -e

rotated_servers=""
for ((i = 0; i < ${server_total_logical_num_for_rotation}; i++))
do
	if [ $i -ne ${bottleneck_serveridx} ]
	then
		if [ "x${rotated_servers}" == "x" ]
		then
			rotated_servers="$i"
		else
			rotated_servers="${rotated_servers}:$i"
		fi
	fi
done

echo "Backup ${DIRNAME}/config.ini into ${DIRNAME}/config.ini.bak, which will be resumed by test/stop_server_rotation.sh"
mv ${DIRNAME}/config.ini ${DIRNAME}/config.ini.bak

echo "Generate new ${DIRNAME}/config.ini based on ${DIRNAME}/configs/config.ini.static.setup to prepare for server rotation"
cp ${DIRNAME}/configs/config.ini.static.setup ${DIRNAME}/config.ini
sed -i '1,$s/workload_name=TODO/workload_name='${workloadname}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_total_logical_num=TODO/server_total_logical_num='${server_total_logical_num_for_rotation}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_total_logical_num_for_rotation=TODO/server_total_logical_num_for_rotation='${server_total_logical_num_for_rotation}/'' ${DIRNAME}/config.ini
sed -i '1,$s/bottleneck_serveridx_for_rotation=TODO/bottleneck_serveridx_for_rotation='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_logical_idxes=TODO0/server_logical_idxes='${bottleneck_serveridx}/'' ${DIRNAME}/config.ini
sed -i '1,$s/server_logical_idxes=TODO1/server_logical_idxes='${rotated_servers}/'' ${DIRNAME}/config.ini
if [ "x${DIRNAME}" == "xfarreach" ]
then
	sed -i '1,$s/controller_snapshot_period=TODO/controller_snapshot_period='${snapshot_period}/'' ${DIRNAME}/config.ini
fi

echo "Sync new ${DIRNAME}/config.ini to all machines"
source scripts/remote/sync_file.sh ${DIRNAME} config.ini
