if [ "x${is_common_included}" != "x1" ]
then
	source scriptsdist/common.sh
fi

#

# NOTE: before running this script
# (1) you need to finish loading phase by launching nocache switch/server + load_and_backup.sh
# (2) you need to finish keydump phase by keydump_and_sync.sh
# (3) you need to finish setup phase by sync_file.sh method/config.ini + launching switch of corresponding method with setuping MAT rules

echo "clear tmp files of controller"
rm ${SERVER_ROOTPATH}/${DIRNAME}/tmp_controller_bwcost.out

echo "retrieve both servers back to the state after loading phase"
# ssh ${USER}@${SERVER0} "
rm -r /tmp/${DIRNAME}/*; 
cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker0.db
# ssh ${USER}@${SERVER1} "rm -r /tmp/${DIRNAME}/*; 
cp -r ${BACKUPS_ROOTPATH}/worker0.db /tmp/${DIRNAME}/worker1.db


echo "launch storage servers of ${DIRNAME}"
source scriptsdist/remote/launchservertestbed.sh
#sleep 90s
sleep 5


if [ ${with_controller} -eq 1 ]
then
	echo "pre-admit hot keys"
	cd ${DIRNAME}
	mx h1 ./warmup_client
	cd ..
	sleep 120s
fi

echo "launch clients of ${DIRNAME}"
cd ./benchmark/ycsb
mx h2 python2 ./bin/ycsb run ${DIRNAME} -pi 1 >tmp_client1.out 2>&1 &
sleep 20s
mx h1 python2 ./bin/ycsb run ${DIRNAME} -pi 0 >tmp_client0.out 2>&1
cd ../../

echo "stop storage servers of ${DIRNAME}"
bash scriptsdist/remote/stopservertestbed.sh