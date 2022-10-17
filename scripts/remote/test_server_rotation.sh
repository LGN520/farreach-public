if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

# NOTE: before running this script
# (1) you need to finish loading phase by launching nocache switch/server + load_and_backup.sh
# (2) you need to finish keydump phase by keydump_and_sync.sh
# (3) you need to finish setup phase by prepare_server_rotation.sh + launching switch of corresponding method with setuping MAT rules

##### Part 0 #####

source scripts/remote/test_server_rotation_p0.sh

##### Part 1 #####

echo "[part 1] run single bottleneck server thread"

source scripts/remote/test_server_rotation_p1.sh

##### Part 2 #####

echo "[part 2] run bottleneck server thread + rotated server thread"

for rotateidx in $(seq 0 $(expr ${server_total_logical_num_for_rotation} - 1))
do
	if [ ${rotateidx} -eq ${bottleneck_serveridx} ]
	then
		continue
	fi

	source scripts/remote/test_server_rotation_p2.sh ${rotateidx}

	#read -p "Continue[y/n]: " is_continue
	#if [ ${is_continue}x == nx ]
	#then
	#	exit
	#fi
done

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
