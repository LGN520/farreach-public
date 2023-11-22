
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

#

tmpdir="benchmark/output/upstreambackups"

# Copy client-side backups to current server
echo "Copy client-side backups to server"
cd ..
if [ "x${workloadmode}" == "x0" ]
then
	mkdir -p ${SERVER_ROOTPATH}/${tmpdir}
	rm -r ${SERVER_ROOTPATH}/${tmpdir}/*
	scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static*client0.out ${SERVER_ROOTPATH}/${tmpdir}
	scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static*client1.out ${SERVER_ROOTPATH}/${tmpdir}
elif [ "x${workloadmode}" == "x1" ]
then
	mkdir -p ${SERVER_ROOTPATH}/${tmpdir}
	rm -r ${SERVER_ROOTPATH}/${tmpdir}/*
	scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client0.out ${SERVER_ROOTPATH}/${tmpdir}
	scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client1.out ${SERVER_ROOTPATH}/${tmpdir}
else
	echo "[ERROR] invalid workload mode: ${workloadmode}"
	exit
fi
cd ${DIRNAME}
