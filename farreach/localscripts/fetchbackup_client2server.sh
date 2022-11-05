if [ "x${is_common_included}" != "x1" ]
then
	cd ..
	source scripts/common.sh
	if [ "x${DIRNAME}" != "xfarreach" ]
	then
		echo "[ERROR] you should change DIRNAME as farreach in scripts/common.sh before running fetchbackup_client2server.sh"
		exit
	fi
	cd ${DIRNAME}
fi

#set -x

tmpdir="benchmark/output/upstreambackups"

# Copy client-side backups to current server
echo "Copy client-side backups to server"
cd ..
if [ "x${workloadmode}" == "x0" ]
then
	mkdir -p ${tmpdir}
	rm -r ${tmpdir}/*
	scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static*client0.out ${tmpdir}
	scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static*client1.out ${tmpdir}
elif [ "x${workloadmode}" == "x1" ]
then
	mkdir -p ${tmpdir}
	rm -r ${tmpdir}/*
	scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client0.out ${tmpdir}
	scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client1.out ${tmpdir}
else
	echo "[ERROR] invalid workload mode: ${workloadmode}"
	exit
fi
cd ${DIRNAME}
