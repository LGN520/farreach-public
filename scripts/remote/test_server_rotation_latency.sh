if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x
set -e

if [ $# -ne 0 ]
then
	echo "Usage: bash scripts/remote/test_server_rotation_latency.sh"
	exit
fi

# Get static throughput statistics under server rotation (workloadmode must be 0)

if [ "x${workloadmode}" == "x0" ]
then
	thpt_midstr="static${server_total_logical_num_for_rotation}"
else
	echo "Invalid workloadmode: ${workloadmode}"
	exit
fi

methodname=${DIRNAME}
statisticsdir="${workloadname}-statistics"
filedir=${CLIENT_ROOTPATH}/benchmark/output/${statisticsdir}

localthptfilename="${methodname}-${midstr}-client0.out"
remotethptfilename="${methodname}-${midstr}-client1.out"

localthptfilepath=${filedir}/${localthptfilename}
remotethptfilepath=${filedir}/${remotethptfilename}

echo "copy thpt statistics file ${remotethptfilename} from another client"
scp ${USER}@${SECONDARY_CLIENT}:${remotethptfilepath} ${filedir}

cd scripts/local/
perrotation_targets=($(python calculate_target_helper.py ${localfilepath} ${remotefilepath} ${bottleneck_serveridx}))
echo "Per-rotation targets: ${perrotation_targets[*]}"
cd ../../

set +e

# TODO: invoke test_server_rotation_latency_<p0/p1/p2>.sh (similar as test_server_rotation.sh)
# NOTE: pass perrotation_targets[i] to the i-th rotation when starting clients
