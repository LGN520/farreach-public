set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

#
#set -e

if [ $# -ne 0 ]
then
	echo "Usage: bash scriptsbmv2/remote/load_and_backup.sh"
	exit
fi

# NOTE: before running this script
# (1) you need to finish setup phase by sync_file.sh nocache/config.ini + launching nocache switch with setuping MAT rules

if [ "x${DIRNAME}" != "xnocache" ]
then
	echo "[ERROR] DIRNAME (${DIRNAME}) is not nocache in scriptsbmv2/common.sh; please update and sync scriptsbmv2/common.sh, and re-launch switch"
	exit
fi


echo "clear files in /tmp/${DIRNAME}"
# ssh ${USER}@${SERVER0} "
rm -rf /tmp/${DIRNAME}/*

echo "launch storage servers of ${DIRNAME}"
source scriptsbmv2/remote/launchservertestbed.sh
sleep 10s

echo "Launch recordload client"
cd benchmark/ycsb
mx h1 python2 ./bin/ycsb load recordload
cd ../../

echo "Sleep 10 minutes to wait for servers to finish compression operations..."
# sleep 10m
# 
echo "stop storage servers of ${DIRNAME}"
source scriptsbmv2/remote/stopservertestbed.sh

echo "backup files from /tmp/${DIRNAME} to ${BACKUPS_ROOTPATH} in each storage server"
# ssh ${USER}@${SERVER0} "
mkdir -p ${BACKUPS_ROOTPATH}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/
# ssh ${USER}@${SERVER1} "mkdir -p ${BACKUPS_ROOTPATH}"
# ssh ${USER}@${SERVER0} "scp -i /root/${CONNECTION_PRIVATEKEY} -r ${BACKUPS_ROOTPATH}/* ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}"

#ssh ${USER}@${SERVER0} "mkdir -p ${BACKUPS_ROOTPATH}/${serverscale}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/${serverscale}/"
#ssh ${USER}@${SERVER1} "mkdir -p ${BACKUPS_ROOTPATH}/${serverscale}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/${serverscale}/"

#if [ "x${workloadmode}" == "x0" ]
#then
#	echo "sync files for server rotation under static pattern"
#	scp -r ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}/${serverscale}/worker${bottleneck_serveridx}.* ${USER}@${SERVER0}:${BACKUPS_ROOTPATH}/${serverscale}/ >/dev/null
#	for ((i = 0; i < ${serverscale}; i++))
#	do
#		if [ "x${i}" != "x${bottleneck_serveridx}" ]
#		then
#			scp -r ${USER}@${SERVER0}:${BACKUPS_ROOTPATH}/${serverscale}/worker${i}.* ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}/${serverscale}/ >/dev/null
#		fi
#	done
#fi

echo "resume and sync original nocache/config.ini"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini

echo "[WARNING] if you are cooperated with other uses, please change the permission of ${BACKUPS_ROOTPATH} to all users in each physical server!"
