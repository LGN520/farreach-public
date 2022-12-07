#!/bin/bash
# run this scripts on dl11 (main client)
# exp_key_distribution

source scripts/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scripts/exps/run_exp_key_distribution.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp5_server_scale="16"
exp5_server_scale_bottleneck="14"
exp5_method_list=("farreach" "netcache" "nocache")
exp5_workload_list=("skewness-90" "skewness-95")
exp5_existed_workload_list=("synthetic")
exp5_backup_workload_list=("skewness-99")

### Create json backup directory
mkdir -p ${EVALUATION_OUTPUT_PREFIX}/exp5/${roundnumber}/

for exp5_method in ${exp5_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp5_method}\"/" ${CLIENT_ROOTPATH}/scripts/common.sh
  cd ${CLIENT_ROOTPATH}/
  bash scripts/remote/sync_file.sh scripts common.sh

  for exp5_workload in ${exp5_workload_list[@]}; do
    echo "[exp5][${exp5_method}][${exp5_workload}]"

    ### Preparation
    echo "[exp5][${exp5_method}][${exp5_workload}] run workload with $exp5_workload" servers
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp5_method}; bash localscripts/stopswitchtestbed.sh"
    
    echo "[exp5][${exp5_method}][${exp5_workload}] update ${exp5_method} config with ${exp5_workload}"
    cp ${CLIENT_ROOTPATH}/${exp5_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp5_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp5_workload}"/" ${CLIENT_ROOTPATH}/${exp5_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp5_server_scale}"/" ${CLIENT_ROOTPATH}/${exp5_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp5_server_scale}"/" ${CLIENT_ROOTPATH}/${exp5_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp5_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp5_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp5_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp5_method}/config.ini


    cd ${CLIENT_ROOTPATH}
    echo "[exp5][${exp5_method}][${exp5_workload}] prepare server rotation" 
    bash scripts/remote/prepare_server_rotation.sh

    echo "[exp5][${exp5_method}][${exp5_workload}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp5_method}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp5_method}; bash localscripts/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp5][${exp5_method}][${exp5_workload}] test server rotation" 
    bash scripts/remote/test_server_rotation.sh

    echo "[exp5][${exp5_method}][${exp5_workload}] stop server rotation"
    bash scripts/remote/stop_server_rotation.sh

    echo "[exp5][${exp5_method}][${exp5_workload}] sync json file and calculate"
    bash scripts/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp5_workload}-statistics/${exp5_method}-static${exp5_server_scale}-client0.out  ${EVALUATION_OUTPUT_PREFIX}/exp5/${roundnumber}/${exp5_workload}-${exp5_method}-static${exp5_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp5_workload}-statistics/${exp5_method}-static${exp5_server_scale}-client1.out  ${EVALUATION_OUTPUT_PREFIX}/exp5/${roundnumber}/${exp5_workload}-${exp5_method}-static${exp5_server_scale}-client1.out 
    echo "[exp5][${exp5_method}][${exp5_workload}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp5_method}; bash localscripts/stopswitchtestbed.sh"
  done
  
  ### Backup for generated workloads
  existed=0
  for exp5_existed_workload in ${exp5_existed_workload_list[@]}; do
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp5_existed_workload}-statistics/${exp5_method}-static${exp5_server_scale}-client0.out  ${EVALUATION_OUTPUT_PREFIX}/exp5/${roundnumber}/${exp5_backup_workload_list[${existed}]}-${exp5_method}-static${exp5_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp5_existed_workload}-statistics/${exp5_method}-static${exp5_server_scale}-client1.out  ${EVALUATION_OUTPUT_PREFIX}/exp5/${roundnumber}/${exp5_backup_workload_list[${existed}]}-${exp5_method}-static${exp5_server_scale}-client1.out 
    existed=$((++existed))
  done
done


