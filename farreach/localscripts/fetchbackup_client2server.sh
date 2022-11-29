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
	mkdir -p ${SERVER_ROOTPATH}/${tmpdir}
	rm -r ${SERVER_ROOTPATH}/${tmpdir}/*
	scp ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static${server_total_logical_num_for_rotation}*client0.out ${SERVER_ROOTPATH}/${tmpdir}
	scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static${server_total_logical_num_for_rotation}client1.out ${SERVER_ROOTPATH}/${tmpdir}
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

# Copy in-switch snapshot id/data from controller in dl16 to secondary server(s) in dl13
tmp_curserver=$(hostname)
if [ "x${tmp_curserver}" == "x${SERVER1}" ]
then
	echo "Copy in-switch snapshot from controller to ${tmp_curserver}"
	mkdir -p /tmp/${DIRNAME}
	rm /tmp/${DIRNAME}/controller.snapshot*
	# TODO: add -i <keypath_atdl13_fromdl16> if necessary
	scp ${USER}@${SERVER0}:/tmp/${DIRNAME}/controller.snapshot* /tmp/${DIRNAME}
fi
