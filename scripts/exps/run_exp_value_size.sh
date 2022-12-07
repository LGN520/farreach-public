#!/bin/bash
# run this scripts on dl11 (main client)
# exp_value_size

source scripts/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scripts/exps/run_exp_value_size.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp6_server_scale="16"
exp6_server_scale_bottleneck="14"
exp6_method_list=("farreach" "netcache" "nocache")
exp6_workload_list=("valuesize-16" "valuesize-32" "valuesize-64")
exp6_existed_workload_list=("synthetic")
exp6_backup_workload_list=("valuesize-128")

### Create json backup directory
mkdir -p ${EVALUATION_OUTPUT_PREFIX}/exp6/${roundnumber}/

for exp6_method in ${exp6_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp6_method}\"/" ${CLIENT_ROOTPATH}/scripts/common.sh
  cd ${CLIENT_ROOTPATH}/
  bash scripts/remote/sync_file.sh scripts common.sh

  for exp6_workload in ${exp6_workload_list[@]}; do
    ### Preparation
    echo "[exp6][${exp6_method}][${exp6_workload}] run workload with $exp6_workload" servers
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp6_method}; bash localscripts/stopswitchtestbed.sh"
    
    echo "[exp6][${exp6_method}][${exp6_workload}] update ${exp6_method} config with ${exp6_workload}"
    cp ${CLIENT_ROOTPATH}/${exp6_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp6_workload}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp6_server_scale}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp6_server_scale}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp6_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini

    cd ${CLIENT_ROOTPATH}
    echo "[exp6][${exp6_method}][${exp6_workload}] prepare server rotation" 
    bash scripts/remote/prepare_server_rotation.sh

    echo "[exp6][${exp6_method}][${exp6_workload}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp6_method}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp6_method}; bash localscripts/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp6][${exp6_method}][${exp6_workload}] test server rotation" 
    bash scripts/remote/test_server_rotation.sh

    echo "[exp6][${exp6_method}][${exp6_workload}] stop server rotation"
    bash scripts/remote/stop_server_rotation.sh

    echo "[exp6][${exp6_method}][${exp6_workload}] sync json file and calculate"
    bash scripts/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client0.out  ${EVALUATION_OUTPUT_PREFIX}/exp6/${roundnumber}/${exp6_workload}-${exp6_method}-static${exp6_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client1.out  ${EVALUATION_OUTPUT_PREFIX}/exp6/${roundnumber}/${exp6_workload}-${exp6_method}-static${exp6_server_scale}-client1.out 
    echo "[exp6][${exp6_method}][${exp6_workload}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${exp6_method}; bash localscripts/stopswitchtestbed.sh"
  done
  
  ### Backup for generated workloads
  existed=0
  for exp6_existed_workload in ${exp6_existed_workload_list[@]}; do
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_existed_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client0.out  ${EVALUATION_OUTPUT_PREFIX}/exp6/${roundnumber}/${exp6_backup_workload_list[${existed}]}-${exp6_method}-static${exp6_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_existed_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client1.out  ${EVALUATION_OUTPUT_PREFIX}/exp6/${roundnumber}/${exp6_backup_workload_list[${existed}]}-${exp6_method}-static${exp6_server_scale}-client1.out 
    existed=$((++existed))
  done
done


