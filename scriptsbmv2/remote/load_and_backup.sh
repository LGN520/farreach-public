if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x
#set -e

if [ $# -ne 0 ]
then
	echo "Usage: bash scripts/remote/load_and_backup.sh"
	exit
fi

# NOTE: before running this script
# (1) you need to finish setup phase by sync_file.sh nocache/config.ini + launching nocache switch with setuping MAT rules

if [ "x${DIRNAME}" != "xnocache" ]
then
	echo "[ERROR] DIRNAME (${DIRNAME}) is not nocache in scripts/common.sh; please update and sync scripts/common.sh, and re-launch switch"
	exit
#else
#	if [ "x${workloadmode}" == "x0" ] && [ "x${server_total_logical_num}" != "x${server_total_logical_num_for_rotation}" ]
#	then
#		echo "[ERROR] server_total_logical_num should = server_total_logical_num_for_rotation under loading phase of static pattern in ${DIRNAME}/config.ini; please update and sync ${DIRNAME}/config.ini, and re-launch switch"
#		exit
#	fi
#	serverscale=${server_total_logical_num}
fi
#echo "workloadmode: ${workloadmode}; server scale: ${serverscale}"

echo "clear files in /tmp/${DIRNAME}"
ssh ${USER}@${SERVER0} "rm -rf /tmp/${DIRNAME}/*"

echo "launch storage servers of ${DIRNAME}"
source scripts/remote/launchservertestbed.sh
sleep 10s

echo "Launch recordload client"
cd benchmark/ycsb
./bin/ycsb load recordload
cd ../../

echo "Sleep 10 minutes to wait for servers to finish compression operations..."
sleep 10m

echo "stop storage servers of ${DIRNAME}"
source scripts/remote/stopservertestbed.sh

echo "backup files from /tmp/${DIRNAME} to ${BACKUPS_ROOTPATH} in each storage server"
ssh ${USER}@${SERVER0} "mkdir -p ${BACKUPS_ROOTPATH}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/"
ssh ${USER}@${SERVER1} "mkdir -p ${BACKUPS_ROOTPATH}"
ssh ${USER}@${SERVER0} "scp -i /home/${USER}/${CONNECTION_PRIVATEKEY} -r ${BACKUPS_ROOTPATH}/* ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}"

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
bash scripts/remote/sync_file.sh ${DIRNAME} config.ini

echo "[WARNING] if you are cooperated with other uses, please change the permission of ${BACKUPS_ROOTPATH} to all users in each physical server!"
