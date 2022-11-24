if [ "x${is_common_included}" != "x1" ]
then
	cd ..
	source scripts/common.sh
	if [ "x${DIRNAME}" != "xfarreach" ]
	then
		echo "[ERROR] you should change DIRNAME as farreach in scripts/common.sh before running fetchsnapshotandmaxseq_controllerandserver2switch.sh"
		exit
	fi
	cd ${DIRNAME}
fi

#set -x

if [ "x${DIRNAME}" != "xfarreach" ]
then
	echo "[ERROR] DIRNAME should be farreach in scripts/common.sh for fetchall_all2switch.sh"
	exit
fi

tmpdir="benchmark/output/upstreambackups"

mkdir -p ${SWITCH_ROOTPATH}/${tmpdir}
rm -r ${SWITCH_ROOTPATH}/${tmpdir}/*
mkdir -p /tmp/${DIRNAME}
rm -r /tmp/${DIRNAME}/*

# Copy client-side backups to switch
echo "Copy client-side backups to switch"
cd ..
if [ "x${workloadmode}" == "x0" ]
then
	scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static${server_total_logical_num_for_rotation}*client0.out ${SWITCH_ROOTPATH}/${tmpdir}
	scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static${server_total_logical_num_for_rotation}*client1.out ${SWITCH_ROOTPATH}/${tmpdir}
elif [ "x${workloadmode}" == "x1" ]
then
	scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client0.out ${SWITCH_ROOTPATH}/${tmpdir}
	scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client1.out ${SWITCH_ROOTPATH}/${tmpdir}
else
	echo "[ERROR] invalid workload mode: ${workloadmode}"
	exit
fi

# Copy in-switch snapshot id/data from controller to switch
echo "Copy in-switch snapshot from controller to switch"
scp ${USER}@${SERVER0}:/tmp/${DIRNAME}/controller.snapshot* /tmp/${DIRNAME}

# Copy latest/snapshot maxseq from server to switch
echo "Copy maxseq files from server to switch"
scp ${USER}@${SERVER0}:/tmp/${DIRNAME}/*maxseq* /tmp/${DIRNAME}
scp ${USER}@${SERVER1}:/tmp/${DIRNAME}/*maxseq* /tmp/${DIRNAME}
