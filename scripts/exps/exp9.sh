# NOTE: exp.sh should be independent with scripts/common.sh, as it could change settings in scripts/common.sh, e.g., change method name

if [ "x${is_global_included}" != "x1" ]
then
	source scripts/global.sh
fi

#set -x
#set -e

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/exps/exp10.sh <roundidx>"
	exit
else
	roundidx=$1
fi

# Global variables and utils

exp_workloadmode=0
exp_rotationscale=16

exp_dirname="benchmark/output/round${roundidx}/exp10/"
mkdir -p ${exp_dirname}
echo "Backup path of generated files: ${exp_dirname}"

exp_configfile="farreach/config.ini"
exp_configfile_bak="farreach/config.ini.exp10"
echo "Backup existing ${exp_configfile} to ${exp_configfile_bak}"
cp ${exp_configfile} ${exp_configfile_bak}

exp_commonfile="scripts/common.sh"
exp_commonfile_bak="scripts/common.sh.exp10"
exp_commonfile_template="scripts/common.temp"
echo "Backup existing ${exp_commonfile} to ${exp_commonfile_bak}"
cp ${exp_commonfile} ${exp_commonfile_bak}
echo "Use farreach as DIRNAME in ${exp_commonfile}"
cp ${exp_commonfile_template} ${exp_commonfile}
sed -i '1,$s/DIRNAME=TODO/DIRNAME=farreach/g' ${exp_commonfile}

function readini() {
	tmpfile=$1
	result=$(awk -F '=' -v tmpsection=[$2] -v tmpkey=$3 '$0==tmpsection {flag = 1; next} /^\[/ {flag = 0; next} flag && $1==tmpkey {print $2}' ${tmpfile})
	echo ${result}
}

function runexp() {
	tmpexpcachesize=$1
	tmpoldcachesize=$2
	echo "[exp10] Run exp10 with cache size of ${tmpexpcachesize}"
	sed -i '1,$s/switch_kv_bucket_num='${tmpoldcachesize}'/switch_kv_bucket_num='${tmpexpcachesize}/'' ${exp_configfile}

	# TODO: NOT work now, as we do NOT launch and configure switch in test_server_rotation.sh now
	# TODO: uncomment after add automatic launch and configure into test_server_rotation.sh
	#echo "Run server rotation for FarReach to get files for recovery"
	#bash scripts/remote/test_server_rotation.sh

	tmpexpdirname=${exp_dirname}/cachesize-${tmpexpcachesize}
	mkdir -p ${tmpexpdirname}

	echo "Backup recovery files to ${tmpexpdirname}"
	#cp benchmark/output/upstreambackups/* ${tmpexpdirname}
	#scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/* ${tmpexpdirname}
	if [ ${exp_workloadmode} -eq 0 ]
	then
		cp benchmark/output/upstreambackups/static${exp_rotationscale}*client0.out ${tmpexpdirname}
		scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/static${exp_rotationscale}*client1.out ${tmpexpdirname}
	elif [ ${exp_workloadmode} -eq 1 ]
	then
		cp benchmark/output/upstreambackups/dynamic-client0.out ${tmpexpdirname}
		scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/dynamic-client1.out ${tmpexpdirname}
	else
		echo "[ERROR] invalid workload mode: ${exp_workloadmode}"
		exit
	fi
	scp ${USER}@${SERVER0}:/tmp/farreach/controller.snapshot* ${tmpexpdirname}
	scp ${USER}@${SERVER0}:/tmp/farreach/*maxseq* ${tmpexpdirname}
	scp ${USER}@${SERVER1}:/tmp/farreach/*maxseq* ${tmpexpdirname}

	echo "Get recovery time"
	bash scripts/remote/test_recovery_time.sh >tmp_test_recovery_time.out 2>&1
	echo "Please record the collect time dumped by test_recovery_time.sh manually"
	echo "Please calculate the average switch recovery time based on tmp_switchos.out manually"
	echo "Please calculate the average switch preprocessing time and recovery time based on tmp_server.out manally"

	echo "Backup statistics files to ${tmpexpdirname}"
	cp tmp_test_recovery_time.out ${tmpexpdirname}
	scp -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1:${SWITCH_ROOTPATH}/farreach/tmp_switchos.out ${tmpexpdirname}
	scp ${USER}@${SERVER0}:${SERVER_ROOTPATH}/farreach/tmp_server.out ${tmpexpdirname}/tmp_server0.out
	scp ${USER}@${SERVER1}:${SERVER_ROOTPATH}/farreach/tmp_server.out ${tmpexpdirname}/tmp_server1.out
}

# exp10-0

exp_cachesize=10000
old_cachesize=$(readini ${exp_configfile_bak} "switch" "switch_kv_bucket_num")
runexp ${exp_cachesize} ${old_cachesize}

# exp10-1
old_cachesize=${exp_cachesize}
exp_cachesize=1000
runexp ${exp_cachesize} ${old_cachesize}

# exp10-1
old_cachesize=${exp_cachesize}
exp_cachesize=100
runexp ${exp_cachesize} ${old_cachesize}

# end of exp10

echo "Retrieve ${exp_configfile_bak} to ${exp_configfile} if any"
mv ${exp_configfile_bak} ${exp_configfile}
echo "Retrieve ${exp_commonfile_bak} to ${exp_commonfile} if any"
mv ${exp_commonfile_bak} ${exp_commonfile}
