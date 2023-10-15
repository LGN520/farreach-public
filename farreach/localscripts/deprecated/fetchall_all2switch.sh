set -x
if [ "x${is_common_included}" != "x1" ]
then
	cd ..
	source scriptsbmv2/common.sh
	if [ "x${DIRNAME}" != "xfarreach" ]
	then
		echo "[ERROR] you should change DIRNAME as farreach in scriptsbmv2/common.sh before running fetchsnapshotandmaxseq_controllerandserver2switch.sh"
		exit
	fi
	cd ${DIRNAME}
fi

#

tmpdir="benchmark/output/upstreambackups"

su ${USER} -c "mkdir -p ${SWITCH_ROOTPATH}/${tmpdir}"
su ${USER} -c "rm -r ${SWITCH_ROOTPATH}/${tmpdir}/*"
su ${USER} -c "mkdir -p /tmp/${DIRNAME}"
su ${USER} -c "rm -r /tmp/${DIRNAME}/*"

# Copy client-side backups to switch
echo "Copy client-side backups to switch"
cd ..
if [ "x${workloadmode}" == "x0" ]
then
	su ${USER} -c "scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static*client0.out ${SWITCH_ROOTPATH}/${tmpdir}"
	su ${USER} -c "scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static*client1.out ${SWITCH_ROOTPATH}/${tmpdir}"
elif [ "x${workloadmode}" == "x1" ]
then
	su ${USER} -c "scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client0.out ${SWITCH_ROOTPATH}/${tmpdir}"
	su ${USER} -c "scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client1.out ${SWITCH_ROOTPATH}/${tmpdir}"
else
	echo "[ERROR] invalid workload mode: ${workloadmode}"
	exit
fi

# Copy in-switch snapshot id/data from controller to switch
echo "Copy in-switch snapshot from controller to switch"
su ${USER} -c "scp ${USER}@${SERVER0}:/tmp/${DIRNAME}/controller.snapshot* /tmp/${DIRNAME}"

# Copy latest/snapshot maxseq from server to switch
echo "Copy maxseq files from server to switch"
su ${USER} -c "scp ${USER}@${SERVER0}:/tmp/${DIRNAME}/*maxseq* /tmp/${DIRNAME}"
su ${USER} -c "scp ${USER}@${SERVER1}:/tmp/${DIRNAME}/*maxseq* /tmp/${DIRNAME}"
