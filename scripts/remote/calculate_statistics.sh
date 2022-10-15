if [ ${is_common_included} -ne 1 ]
then
	source scripts/common.sh
fi

set -x
set -e

if [ $# -ne 0 ]
then
	echo "Usage: bash calculate_statistics.sh"
	exit
fi

if [ ${workloadmode} == "0" ]
then
	midstr="static${server_total_logical_num_for_rotation}"
elif [ ${workloadmode} == "1" ]
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

cd scripts/local/
python calculate_statistics_helper.py ${workloadpattern} ${localfilepath} ${remotefilepath}
cd ../../
