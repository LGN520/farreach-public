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

function getTiming(){
    start=$1
    end=$2
 
    start_s=`echo $start | cut -d '.' -f 1`
    start_ns=`echo $start | cut -d '.' -f 2`
    end_s=`echo $end | cut -d '.' -f 1`
    end_ns=`echo $end | cut -d '.' -f 2`
 
    time_micro=$(( (10#$end_s-10#$start_s)*1000000 + (10#$end_ns/1000 - 10#$start_ns/1000) ))
    #time_ms=`expr $time_micro/1000  | bc `
 
    #echo "$time_micro microseconds"
    #echo "$time_ms ms"
	echo $(echo "scale=4; ${time_micro} / 1000.0 / 1000.0" | bc)
}

#set -x

tmpdir="benchmark/output/upstreambackups"

# Copy client-side backups to current server
echo "Copy client-side backups to server"
cd ..
begin_time_1=`date +%s.%N`
if [ "x${workloadmode}" == "x0" ]
then
	mkdir -p ${SERVER_ROOTPATH}/${tmpdir}
	rm -r ${SERVER_ROOTPATH}/${tmpdir}/*
	# Copy client0 and client1 backup records in parallel
	scp -i /home/${USER}/${CONNECTION_PRIVATEKEY} ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static${server_total_logical_num_for_rotation}-${bottleneck_serveridx}*client0.out ${SERVER_ROOTPATH}/${tmpdir}; scp -i /home/${USER}/${CONNECTION_PRIVATEKEY} ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/static${server_total_logical_num_for_rotation}-${bottleneck_serveridx}*client1.out ${SERVER_ROOTPATH}/${tmpdir}
elif [ "x${workloadmode}" == "x1" ]
then
	mkdir -p ${SERVER_ROOTPATH}/${tmpdir}
	rm -r ${SERVER_ROOTPATH}/${tmpdir}/*
	scp -i /home/${USER}/${CONNECTION_PRIVATEKEY} ${USER}@${MAIN_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client0.out ${SERVER_ROOTPATH}/${tmpdir}
	scp -i /home/${USER}/${CONNECTION_PRIVATEKEY} ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdir}/dynamic-client1.out ${SERVER_ROOTPATH}/${tmpdir}
else
	echo "[ERROR] invalid workload mode: ${workloadmode}"
	exit
fi
end_time_1=`date +%s.%N`
collect_time_1=$(getTiming ${begin_time_1} ${end_time_1})
cd ${DIRNAME}

# Copy in-switch snapshot id/data from controller in dl16 to secondary server(s) in dl13
tmp_curserver=$(hostname)
echo "[Statistics] per-client backups collect time in ${tmp_curserver}: ${collect_time_1} s"
if [ "x${tmp_curserver}" == "x${SERVER1}" ]
then
	echo "Copy in-switch snapshot from controller to ${tmp_curserver}"
	mkdir -p /tmp/${DIRNAME}
	rm /tmp/${DIRNAME}/controller.snapshot*
	# TODO: add -i <keypath_atdl13_fromdl16> if necessary
	scp -i /home/${USER}/${CONNECTION_PRIVATEKEY} ${USER}@${SERVER0}:/tmp/${DIRNAME}/controller.snapshot* /tmp/${DIRNAME}
fi
