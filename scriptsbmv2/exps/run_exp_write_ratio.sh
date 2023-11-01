set -x
#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_write_ratio

source scriptsbmv2/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scriptsbmv2/exps/run_exp_write_ratio.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp4_server_scale=16
exp4_server_scale_bottleneck=4
exp4_method_list=("farreach" "netcache" "nocache")
exp4_workload_list=("synthetic-25" "synthetic-75" "synthetic")
exp4_existed_workload_list=("workloadc" "workloada")
exp4_backup_workload_list=("synthetic-0" "synthetic-50")
exp4_output_path="${EVALUATION_OUTPUT_PREFIX}/exp4/${roundnumber}"

### Create json backup directory
mkdir -p ${exp4_output_path}/

for exp4_method in ${exp4_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp4_method}\"/" ${CLIENT_ROOTPATH}/scriptsbmv2/common.sh
  cd ${CLIENT_ROOTPATH}
  bash scriptsbmv2/remote/sync_file.sh scripts common.sh

  for exp4_workload in ${exp4_workload_list[@]}; do
    echo "[exp4][${exp4_method}][${exp4_workload}]"

    ### Preparation
    echo "[exp4][${exp4_method}][${exp4_workload}] run workload with $exp4_workload" servers
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd /${SWITCH_ROOTPATH}/${exp4_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
    
    echo "[exp4][${exp4_method}][${exp4_workload}] update ${exp4_method} config with ${exp4_workload}"
    cp ${CLIENT_ROOTPATH}/${exp4_method}/configs/config.ini.bmv2 ${CLIENT_ROOTPATH}/${exp4_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp4_workload}"/" ${CLIENT_ROOTPATH}/${exp4_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp4_server_scale}"/" ${CLIENT_ROOTPATH}/${exp4_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp4_server_scale}"/" ${CLIENT_ROOTPATH}/${exp4_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp4_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp4_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp4_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp4_method}/config.ini

    cd ${CLIENT_ROOTPATH}
    echo "[exp4][${exp4_method}][${exp4_workload}] prepare server rotation" 
    bash scriptsbmv2/remote/prepare_server_rotation.sh

    echo "[exp4][${exp4_method}][${exp4_workload}] start switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd /${SWITCH_ROOTPATH}/${exp4_method}/bmv2; nohup python network.py &"
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd /${SWITCH_ROOTPATH}/${exp4_method}; bash localscriptsbmv2/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp4][${exp4_method}][${exp4_workload}] test server rotation" 
    bash scriptsbmv2/remote/test_server_rotation.sh

    echo "[exp4][${exp4_method}][${exp4_workload}] stop server rotation"
    bash scriptsbmv2/remote/stop_server_rotation.sh

    echo "[exp4][${exp4_method}][${exp4_workload}] sync json file and calculate"
    bash scriptsbmv2/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp4_workload}-statistics/${exp4_method}-static${exp4_server_scale}-client0.out  ${exp4_output_path}/${exp4_workload}-${exp4_method}-static${exp4_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp4_workload}-statistics/${exp4_method}-static${exp4_server_scale}-client1.out  ${exp4_output_path}/${exp4_workload}-${exp4_method}-static${exp4_server_scale}-client1.out 
    echo "[exp4][${exp4_method}][${exp4_workload}] stop switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd /${SWITCH_ROOTPATH}/${exp4_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
  done
  
  ### Backup for generated workloads
  existed=0
  for exp4_existed_workload in ${exp4_existed_workload_list[@]}; do
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp4_existed_workload}-statistics/${exp4_method}-static${exp4_server_scale}-client0.out  ${exp4_output_path}/${exp4_backup_workload_list[${existed}]}-${exp4_method}-static${exp4_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp4_existed_workload}-statistics/${exp4_method}-static${exp4_server_scale}-client1.out  ${exp4_output_path}/${exp4_backup_workload_list[${existed}]}-${exp4_method}-static${exp4_server_scale}-client1.out 
    existed=$((++existed))
  done
done


