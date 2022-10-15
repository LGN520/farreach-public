if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

set -x
set -e

if [ $# -ne 0 ]
then
	echo "Usage: bash scripts/remote/load_and_backup.sh"
	exit
fi

if [ "x${DIRNAME}" != "xnocache" ]
then
	echo "[ERROR][scripts/common.sh] DIRNAME (${DIRNAME}) is not nocache in scripts/common.sh"
	exit
else
	if [ "x${workloadmode}" == "x0" ] && [ "x${server_total_logical_num}" != "x${server_total_logical_num_for_rotation}" ]
	then
		echo "[ERROR][${DIRNAME}/config.ini] server_total_logical_num should = server_total_logical_num_for_rotation under loading phase of static pattern"
		exit
	fi
	serverscale=${server_total_logical_num}
fi
echo "workloadmode: ${workloadmode}; server scale: ${serverscale}"

cd benchmark/ycsb
./bin/ycsb load recordload
cd ../../
bash scripts/remote/stopservertestbed.sh

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
