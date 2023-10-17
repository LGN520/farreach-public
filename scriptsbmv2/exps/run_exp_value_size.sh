set -x
#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_value_size

source scriptsbmv2/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scriptsbmv2/exps/run_exp_value_size.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp6_server_scale="16"
exp6_server_scale_bottleneck="14"
exp6_method_list=("farreach" "netcache" "nocache")
exp6_workload_list=("valuesize-16" "valuesize-32" "valuesize-64")
exp6_existed_workload_list=("synthetic")
exp6_backup_workload_list=("valuesize-128")
exp6_output_path="${EVALUATION_OUTPUT_PREFIX}/exp6/${roundnumber}"

### Create json backup directory
mkdir -p ${exp6_output_path}/

for exp6_method in ${exp6_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp6_method}\"/" ${CLIENT_ROOTPATH}/scriptsbmv2/common.sh
  cd ${CLIENT_ROOTPATH}/
  bash scriptsbmv2/remote/sync_file.sh scripts common.sh

  for exp6_workload in ${exp6_workload_list[@]}; do
    ### Preparation
    echo "[exp6][${exp6_method}][${exp6_workload}] run workload with $exp6_workload" servers
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp6_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
    
    echo "[exp6][${exp6_method}][${exp6_workload}] update ${exp6_method} config with ${exp6_workload}"
    cp ${CLIENT_ROOTPATH}/${exp6_method}/configs/config.ini.bmv2 ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp6_workload}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp6_server_scale}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp6_server_scale}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp6_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp6_method}/config.ini

    cd ${CLIENT_ROOTPATH}
    echo "[exp6][${exp6_method}][${exp6_workload}] prepare server rotation" 
    bash scriptsbmv2/remote/prepare_server_rotation.sh

    echo "[exp6][${exp6_method}][${exp6_workload}] start switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp6_method}/bmv2; nohup python network.py &"
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp6_method}; bash localscriptsbmv2/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp6][${exp6_method}][${exp6_workload}] test server rotation" 
    bash scriptsbmv2/remote/test_server_rotation.sh

    echo "[exp6][${exp6_method}][${exp6_workload}] stop server rotation"
    bash scriptsbmv2/remote/stop_server_rotation.sh

    echo "[exp6][${exp6_method}][${exp6_workload}] sync json file and calculate"
    bash scriptsbmv2/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client0.out  ${exp6_output_path}/${exp6_workload}-${exp6_method}-static${exp6_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client1.out  ${exp6_output_path}/${exp6_workload}-${exp6_method}-static${exp6_server_scale}-client1.out 
    echo "[exp6][${exp6_method}][${exp6_workload}] stop switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp6_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
  done
  
  ### Backup for generated workloads
  existed=0
  for exp6_existed_workload in ${exp6_existed_workload_list[@]}; do
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_existed_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client0.out  ${exp6_output_path}/${exp6_backup_workload_list[${existed}]}-${exp6_method}-static${exp6_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp6_existed_workload}-statistics/${exp6_method}-static${exp6_server_scale}-client1.out  ${exp6_output_path}/${exp6_backup_workload_list[${existed}]}-${exp6_method}-static${exp6_server_scale}-client1.out 
    existed=$((++existed))
  done
done


