source scripts/common.sh

set -x
set -e

if [ $# -ne 0 ]
then
	echo "Usage: bash load_and_backup.sh"
	exit
fi

if [ ${DIRNAME} -ne "nocache" ]
then
	echo "[ERROR][scripts/common.sh] DIRNAME is not nocache in scripts/common.sh"
	exit
else
	if [ ${workloadmode} -eq 0 ] && [ ${server_total_logical_num} -ne ${server_total_logial_num_for_rotation} ]
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
ssh ${USER}@${SERVER0} "mkdir -p ${BACKUPS_ROOTPATH}/${serverscale}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/${serverscale}/*"
ssh ${USER}@${SERVER1} "mkdir -p ${BACKUPS_ROOTPATH}/${serverscale}; mv /tmp/${DIRNAME}/* ${BACKUPS_ROOTPATH}/${serverscale}/*"

if [ ${workloadmode} -eq "0" ]
then
	echo "sync files for server rotation under static pattern"
	scp -r ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}/${serverscale}/worker${bottleneck_serveridx}.* ${USER}@${SERVER0}:${BACKUPS_ROOTPATH}/${serverscale}/ >/dev/null
	for ((i = 0; i < ${serverscale}; i++))
	do
		if [ i -ne ${bottleneck_serveridx} ]
		then
			scp -r ${USER}@${SERVER0}:${BACKUPS_ROOTPATH}/${serverscale}/worker${i}.* ${USER}@${SERVER1}:${BACKUPS_ROOTPATH}/${serverscale}/ >/dev/null
		fi
	done
fi
