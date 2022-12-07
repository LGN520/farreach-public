#!/bin/bash
# run this scripts on dl11 (main client)
# exp_scalability

source scripts/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scripts/exps/run_exp_scalability.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp3_workload="workloada"
exp3_method_list=("farreach")
exp3_scalability_list=("32" "64" "128")
exp3_scalability_bottleneck_list=("29" "59" "118")

### Create json backup directory
mkdir -p ${EVALUATION_OUTPUT_PREFIX}/exp3/${roundnumber}/

for exp3_method in ${exp3_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp3_method}\"/" ${CLIENT_ROOTPATH}/scripts/common.sh
  scalability_counter=0
  for exp3_scalability in ${exp3_scalability_list[@]}; do
    echo "[exp3][${exp3_method}][${exp3_scalability}]"

    ### Preparation
    echo "[exp3][${exp3_method}][${exp3_scalability}] run workload with ${exp3_scalability}" servers
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp3_method}; bash localscripts/stopswitchtestbed.sh"
    
    echo "[exp3][${exp3_method}][${exp3_scalability}] update ${exp3_method} config with workloada"
    cp ${CLIENT_ROOTPATH}/${exp3_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp3_workload}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp3_scalability}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp3_scalability}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp3_scalability_bottleneck_list[${scalability_counter}]}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini


    cd ${CLIENT_ROOTPATH}
    echo "[exp3][${exp3_method}][${exp3_scalability}] prepare server rotation" 
    bash scripts/remote/prepare_server_rotation.sh

    echo "[exp3][${exp3_method}][${exp3_scalability}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp3_method}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp3_method}; bash localscripts/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp3][${exp3_method}][${exp3_scalability}] test server rotation" 
    bash scripts/remote/test_server_rotation.sh

    echo "[exp3][${exp3_method}][${exp3_scalability}] stop server rotation"
    bash scripts/remote/stop_server_rotation.sh

    echo "[exp3][${exp3_method}][${exp3_scalability}] sync json file and calculate"
    bash scripts/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static${exp3_scalability}-client0.out  ${EVALUATION_OUTPUT_PREFIX}/exp3/${roundnumber}/${exp3_workload}-${exp3_method}-static${exp3_scalability}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static${exp3_scalability}-client1.out  ${EVALUATION_OUTPUT_PREFIX}/exp3/${roundnumber}/${exp3_workload}-${exp3_method}-static${exp3_scalability}-client1.out 
    echo "[exp3][${exp3_method}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp3_method}; bash localscripts/stopswitchtestbed.sh"
    scalability_counter=$((++scalability_counter))
  done

  cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static16-client0.out  ${EVALUATION_OUTPUT_PREFIX}/exp3/${roundnumber}/${exp3_workload}-${exp3_method}-static16-client0.out 
  cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static16-client1.out  ${EVALUATION_OUTPUT_PREFIX}/exp3/${roundnumber}/${exp3_workload}-${exp3_method}-static16-client1.out 
done
