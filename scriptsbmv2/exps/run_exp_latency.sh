set -x
#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_latency

source scriptsbmv2/global.sh

if [ $# -ne 1 ]; then
	echo "Usage: bash scriptsbmv2/exps/run_exp_latency.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp2_method_list=("farreach" "netcache" "nocache")
exp2_workload="workloada"
exp2_target_thpt_list=("0.2" "0.4" "0.6" "0.8")
exp2_server_scale="16"
exp2_server_scale_bottleneck="14"
exp2_output_directory="${EVALUATION_OUTPUT_PREFIX}/exp2/${roundnumber}"

### Create output directory
mkdir -p ${exp2_output_directory}

# ### THROUPUT
for exp2_method in ${exp2_method_list[@]}; do
  echo "[exp2][${exp2_method}]"

  sed -i "/^DIRNAME=/s/=.*/=\"${exp2_method}\"/" ${CLIENT_ROOTPATH}/scriptsbmv2/common.sh
  cd ${CLIENT_ROOTPATH}
  bash scriptsbmv2/remote/sync_file.sh scripts common.sh

  ### Preparation
  echo "[exp2][${exp2_method}] run workload with $exp2_workload" servers
  ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
  
  echo "[exp2][${exp2_method}] update ${exp2_method} config with ${exp2_workload}"
  cp ${CLIENT_ROOTPATH}/${exp2_method}/configs/config.ini.bmv2 ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
  sed -i "/^workload_name=/s/=.*/="${exp2_workload}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
  sed -i "/^server_total_logical_num=/s/=.*/="${exp2_server_scale}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
  sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp2_server_scale}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
  sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp2_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
  sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
  sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini

  cd ${CLIENT_ROOTPATH}
  echo "[exp2][${exp2_method}] prepare server rotation" 
  bash scriptsbmv2/remote/prepare_server_rotation.sh

  echo "[exp2][${exp2_method}] start switchos" 
  ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}/bmv2; nohup python network.py &"
  ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}; bash localscriptsbmv2/launchswitchostestbed.sh"

  sleep 20s


  ### Evaluation
  echo "[exp2][${exp2_method}] test server rotation" 
  bash scriptsbmv2/remote/test_server_rotation.sh

  echo "[exp2][${exp2_method}] stop server rotation"
  bash scriptsbmv2/remote/stop_server_rotation.sh

  echo "[exp2][${exp2_method}] sync json file and calculate"
  bash scriptsbmv2/remote/calculate_statistics.sh 0


  ### Cleanup
  cp ${CLIENT_ROOTPATH}/benchmark/output/${exp2_workload}-statistics/${exp2_method}-static${exp2_server_scale}-client0.out  ${exp2_output_directory}/${exp2_workload}-${exp2_method}-static${exp2_server_scale}-client0.out 
  cp ${CLIENT_ROOTPATH}/benchmark/output/${exp2_workload}-statistics/${exp2_method}-static${exp2_server_scale}-client1.out  ${exp2_output_directory}/${exp2_workload}-${exp2_method}-static${exp2_server_scale}-client1.out 
  echo "[exp2][${exp2_method}] stop switchos" 
  ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
done


### LATENCY
for exp2_method in ${exp2_method_list[@]}; do
  echo "[exp2][${exp2_method}]"

  sed -i "/^DIRNAME=/s/=.*/=\"${exp2_method}\"/" ${CLIENT_ROOTPATH}/scriptsbmv2/common.sh
  cd ${CLIENT_ROOTPATH}
  bash scriptsbmv2/remote/sync_file.sh scripts common.sh

  for exp2_target_thpt in ${exp2_target_thpt_list[@]}; do
    ### Preparation
    echo "[exp2][${exp2_method}][${exp2_target_thpt}] run workload with $exp2_workload" servers
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}; bash localscriptsbmv2/stopswitchtestbed.sh"

    echo "[exp2][${exp2_method}][${exp2_target_thpt}] update ${exp2_method} config with ${exp2_workload}"
    cp ${CLIENT_ROOTPATH}/${exp2_method}/configs/config.ini.bmv2 ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp2_workload}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp2_server_scale}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp2_server_scale}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp2_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp2_method}/config.ini

    cd ${CLIENT_ROOTPATH}
    echo "[exp2][${exp2_method}][${exp2_target_thpt}] prepare server rotation" 
    bash scriptsbmv2/remote/prepare_server_rotation.sh

    echo "[exp2][${exp2_method}][${exp2_target_thpt}] prepare calculate target"
    sed -i "/^TARGET_AGGTHPT=/s/=.*/="${exp2_target_thpt}"/" scriptsbmv2/local/calculate_target_helper.py

    echo "[exp2][${exp2_method}][${exp2_target_thpt}] start switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}/bmv2; nohup python network.py &"
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}; bash localscriptsbmv2/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp2][${exp2_method}][${exp2_target_thpt}] test server rotation with target throughput" 
    bash scriptsbmv2/remote/test_server_rotation_latency.sh

    echo "[exp2][${exp2_method}][${exp2_target_thpt}] stop server rotation"
    bash scriptsbmv2/remote/stop_server_rotation.sh

    echo "[exp2][${exp2_method}][${exp2_target_thpt}] sync json file and calculate"
    bash scriptsbmv2/remote/calculate_statistics.sh 1

    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp2_workload}-statistics/${exp2_method}-latency-static${exp2_server_scale}-client0.out  ${exp2_output_directory}/${exp2_target_thpt}-${exp2_workload}-${exp2_method}-static${exp2_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp2_workload}-statistics/${exp2_method}-latency-static${exp2_server_scale}-client1.out  ${exp2_output_directory}/${exp2_target_thpt}-${exp2_workload}-${exp2_method}-static${exp2_server_scale}-client1.out 
    echo "[exp2][${exp2_method}][${exp2_target_thpt}] stop switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp2_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
  done
done
