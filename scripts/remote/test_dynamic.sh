if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

# NOTE: before running this script
# (1) you need to finish loading phase by launching nocache switch/server + load_and_backup.sh
# (2) you need to finish keydump phase by keydump_and_sync.sh
# (3) you need to finish setup phase by sync_file.sh method/config.ini + launching switch of corresponding method with setuping MAT rules

echo "launch storage servers of ${DIRNAME}"
source scripts/remote/launchservertestebed.sh
sleep 10s

if [ ${with_controller} -eq 1 ]
then
	echo "pre-admit hot keys"
	cd ${DIRNAME}
	./warmup_client
	cd ..
	sleep 10s
fi

echo "launch clients pf ${DIRNAME}"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/benchmark/ycsb/; nohup ./bin/ycsb run ${DIRNAME} -pi 1 >tmp_client.out 2>&1 &"
sleep 5s
cd benchmark/ycsb/
./bin/ycsb run ${DIRNAME} -pi 0
cd ../../

echo "stop storage servers of ${DIRNAME}"
source scripts/remote/stopservertestbed.sh
