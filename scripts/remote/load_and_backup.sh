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
else
	if [ "x${workloadmode}" == "x0" ] && [ "x${server_total_logical_num}" != "x${server_total_logical_num_for_rotation}" ]
	then
		echo "[ERROR] server_total_logical_num should = server_total_logical_num_for_rotation under loading phase of static pattern in ${DIRNAME}/config.ini; please update and sync ${DIRNAME}/config.ini, and re-launch switch"
		exit
	fi
	serverscale=${server_total_logical_num}
fi
echo "workloadmode: ${workloadmode}; server scale: ${serverscale}"

echo "launch storage servers of ${DIRNAME}"
source scripts/remote/launchservertestbed.sh
sleep 10s

echo "Launch recordload client"
cd benchmark/ycsb
./bin/ycsb load recordload
cd ../../

echo "stop storage servers of ${DIRNAME}"
source scripts/remote/stopservertestbed.sh

echo "backup files from /tmp/${DIRNAME} to ${BACKUPS_ROOTPATH} in each physical server"
ssh ${USER}@${SERVER0} "mkdir -p ${BACKUPS_ROOTPATH}/${serverscale}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/${serverscale}/"
ssh ${USER}@${SERVER1} "mkdir -p ${BACKUPS_ROOTPATH}/${serverscale}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/${serverscale}/"

if [ "x${workloadmode}" == "x0" ]
then
	echo "sync files for server rotation under static pattern"
	scp -r ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}/${serverscale}/worker${bottleneck_serveridx}.* ${USER}@${SERVER0}:${BACKUPS_ROOTPATH}/${serverscale}/ >/dev/null
	for ((i = 0; i < ${serverscale}; i++))
	do
		if [ "x${i}" != "x${bottleneck_serveridx}" ]
		then
			scp -r ${USER}@${SERVER0}:${BACKUPS_ROOTPATH}/${serverscale}/worker${i}.* ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}/${serverscale}/ >/dev/null
		fi
	done
fi

echo "[WARNING] if you are cooperated with other uses, please change the permission of ${BACKUPS_ROOTPATH} to all users in each physical server!"
