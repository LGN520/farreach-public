#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# only to run this script when single rotation is needed
# Pre-requisite:
#		All clients, servers and switchs are configured to be running static workload
# Applies for experiments:
# 	- exp_key_distribution
# 	- exp_latency
# 	- exp_scalability
# 	- exp_value_size
# 	- exp_write_ratio
# 	- exp_ycsb

source scripts/global.sh
if [ $# -lt 7 ]; then
	echo "Usage: bash scripts/exps/run_makeup_rotation_exp.sh <expname> <roundnumber> <methodname> <workloadname> <serverscale> <bottleneckidx> <targetrotation> [targetthpt]"
	exit
fi
expname=$1
roundnumber=$2
methodname=$3
workloadname=$4
serverscale=$5
bottleneckidx=$6
targetrotation=$7
targetthpt=-1

if [ $# -eq 8 ]; then
	targetthpt=$8
fi
tmptargetthpt=${tmptargetthpt%.*}

exp_makeup_output_path="${EVALUATION_OUTPUT_PREFIX}/${expname}/${roundnumber}"

### Recover corresponding output files
cp ${exp_makeup_output_path}/${workloadname}-${methodname}-static${serverscale}-client0.out ${CLIENT_ROOTPATH}/benchmark/output/${workloadname}-statistics/${methodname}-static${serverscale}-client0.out  
scp ${exp_makeup_output_path}/${workloadname}-${methodname}-static${serverscale}-client1.out ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/${workloadname}-statistics/${methodname}-static${serverscale}-client1.out  

### Stop running switch
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${methodname}; bash localscripts/stopswitchtestbed.sh"

### Prepare config file
echo "[expmakeup][${methodname}][${workloadname}] update ${methodname} config with ${workloadname}"
cp ${CLIENT_ROOTPATH}/${methodname}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${methodname}/config.ini
sed -i "/^workload_name=/s/=.*/="${workloadname}"/" ${CLIENT_ROOTPATH}/${methodname}/config.ini
sed -i "/^server_total_logical_num=/s/=.*/="${serverscale}"/" ${CLIENT_ROOTPATH}/${methodname}/config.ini
sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${serverscale}"/" ${CLIENT_ROOTPATH}/${methodname}/config.ini
sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${methodname}/config.ini
sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${methodname}/config.ini
sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${bottleneckidx}"/" configs/${methodname}-config.ini

cd ${CLIENT_ROOTPATH}
echo "[expmakeup][${methodname}][${workloadname}] prepare server rotation" 
bash scripts/remote/prepare_server_rotation.sh

echo "[expmakeup][${methodname}][${workloadname}] start switchos" 
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${methodname}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${methodname}; bash localscripts/launchswitchostestbed.sh"

sleep 20s

### Evaluation
echo "[expmakeup][${methodname}][${workloadname}] test server rotation" 
if [ $# -eq 8 ]; then
	bash scripts/remote/test_server_rotation_p2.sh 1 targetrotation targetthpt
else
	bash scripts/remote/test_server_rotation_p2.sh 1 targetrotation
fi

echo "[expmakeup][${methodname}][${workloadname}] stop server rotation"
bash scripts/remote/stop_server_rotation.sh

echo "[expmakeup][${methodname}][${workloadname}] sync json file and calculate"
if [ $# -eq 8 ]; then
	bash scripts/remote/calculate_statistics.sh 1
else
	bash scripts/remote/calculate_statistics.sh 0
fi

### Cleanup
cp ${CLIENT_ROOTPATH}/benchmark/output/${workloadname}-statistics/${methodname}-static${serverscale}-client0.out  ${exp_makeup_output_path}/${workloadname}-${methodname}-static${serverscale}-client0.out 
cp ${CLIENT_ROOTPATH}/benchmark/output/${workloadname}-statistics/${methodname}-static${serverscale}-client1.out  ${exp_makeup_output_path}/${workloadname}-${methodname}-static${serverscale}-client1.out 
echo "[[expmakeup][${methodname}][${workloadname}] stop switchos" 
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${methodname}; bash localscripts/stopswitchtestbed.sh"
