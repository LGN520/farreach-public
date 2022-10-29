if [ "x${is_common_included}" != "x1" ]
then
	source scripts-inmemory/common.sh
fi

#set -x
set -e

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts-inmemory/remote/calculate_statistics.sh <isCalculatingLatency>"
	exit
fi

isLatency=$1

if [ "x${workloadmode}" == "x0" ]
then
	if [ "x${isLatency}" == "x0" ]; then
		midstr="static${server_total_logical_num_for_rotation}"
	elif [ "x${isLatency}" == "x1" ]; then
		midstr="latency-static${server_total_logical_num_for_rotation}"
	else
		echo "Invalid isLatency: ${isLatency}"
	fi
elif [ "x${workloadmode}" == "x1" ]
then
	midstr=${dynamicpattern}
else
	echo "Invalid workloadmode: ${workloadmode}"
	exit
fi

methodname=${DIRNAME}
statisticsdir="${workloadname}-statistics"
filedir=${CLIENT_ROOTPATH}/benchmark/output/${statisticsdir}

localfilename="${methodname}-${midstr}-client0.out"
remotefilename="${methodname}-${midstr}-client1.out"

localfilepath=${filedir}/${localfilename}
remotefilepath=${filedir}/${remotefilename}

echo "copy statistics file ${remotefilename} from another client"
scp ${USER}@${SECONDARY_CLIENT}:${remotefilepath} ${filedir}

cd scripts-inmemory/local/
python calculate_statistics_helper.py ${workloadmode} ${localfilepath} ${remotefilepath} ${bottleneck_serveridx}
cd ../../
