source ../common.sh

set -x
set -e

if [ $# -ne 3 ]
then
	echo "Usage: bash calculate_statistics.sh <workloadname> <static/dynamic> <static-scale/dynamic-pattern>"
	echo "Static example: bash calculate_statistics.sh workloada static 16"
	echo "Dynamic example: bash calculate_statistics.sh workloada dynamic hotin"
	exit
fi

workloadname=$1
workloadpattern=$2
if [ ${workloadpattern} == "static" ]
then
	midstr="static$3"
elif [ ${workloadpattern} == "dynamic" ]
then
	midstr=$3
else
	echo "Invalid argument 2: $2, which should be static or dynamic"
	exit
fi

methodname=${DIRNAME}
statisticsdir="${workloadname}-statistics"
filedir=~/${CLIENT_ROOTPATH}/benchmark/output/${statisticsdir}

localfilename="${methodname}-${midstr}-client0.out"
remotefilename="${methodname}-${midstr}-client1.out"

localfilepath=${filedir}/${localfilename}
remotefilepath=${filedir}/${remotefilename}

echo "copy statistics file ${remotefilename} from another client"
scp ${USER}@${SECONDARY_CLIENT}:${remotefilepath} ${filedir}

python calculate_statistics_helper.py ${workloadpattern} ${localfilepath} ${remotefilepath}
