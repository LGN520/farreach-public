#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_ycsb

source scripts/global.sh

if [ $# -ne 1 ]; then
	echo "Usage: bash scripts/exps/run_exp1.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp1_method_list=("farreach" "netcache" "nocache")
exp1_core_workload_list=("workloada" "workloadb" "workloadc" "workloadd" "workloadf" "workload-load")
exp1_server_scale="16"
exp1_output_path="${EVALUATION_OUTPUT_PREFIX}/exp1/${roundnumber}"

### Create json backup directory
mkdir -p ${exp1_output_path}/

for exp1_method in ${exp1_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp1_method}\"/" ${CLIENT_ROOTPATH}/scripts/common.sh
  cd ${CLIENT_ROOTPATH}
  bash scripts/remote/sync_file.sh scripts common.sh

  for exp1_workload in ${exp1_core_workload_list[@]}; do
    echo "[exp1][${exp1_method}][${exp1_workload}]"

    ### Preparation
    echo "[exp1][${exp1_method}][${exp1_workload}] run workload with $exp1_workload" servers
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp1_method}; bash localscripts/stopswitchtestbed.sh"
    
    echo "[exp1][${exp1_method}][${exp1_workload}] update ${exp1_method} config with ${exp1_workload}"
    cp ${CLIENT_ROOTPATH}/${exp1_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp1_workload}"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp1_server_scale}"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp1_server_scale}"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    if [ "x${exp1_workload}" == "xworkload-load" ]; then
      sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="13"/" configs/${exp1_method}-config.ini
    elif [ "x${exp1_workload}" == "xworkloadd" ]; then
      sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="15"/" configs/${exp1_method}-config.ini
    else
      sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="14"/" configs/${exp1_method}-config.ini
    fi

    cd ${CLIENT_ROOTPATH}
    echo "[exp1][${exp1_method}][${exp1_workload}] prepare server rotation" 
    bash scripts/remote/prepare_server_rotation.sh

    echo "[exp1][${exp1_method}][${exp1_workload}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp1_method}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp1_method}; bash localscripts/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp1][${exp1_method}][${exp1_workload}] test server rotation" 
    bash scripts/remote/test_server_rotation.sh

    echo "[exp1][${exp1_method}][${exp1_workload}] stop server rotation"
    bash scripts/remote/stop_server_rotation.sh

    echo "[exp1][${exp1_method}][${exp1_workload}] sync json file and calculate"
    bash scripts/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp1_workload}-statistics/${exp1_method}-static${exp1_server_scale}-client0.out  ${exp1_output_path}/${exp1_workload}-${exp1_method}-static${exp1_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp1_workload}-statistics/${exp1_method}-static${exp1_server_scale}-client1.out  ${exp1_output_path}/${exp1_workload}-${exp1_method}-static${exp1_server_scale}-client1.out 
    echo "[exp1][${exp1_method}][${exp1_workload}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp1_method}; bash localscripts/stopswitchtestbed.sh"
  done
done


