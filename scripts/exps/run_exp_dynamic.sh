#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_dynamic

source scripts/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scripts/exps/run_exp_dynamic.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp7_workload="synthetic"
exp7_server_scale=2
exp7_server_scale_for_rotation=16
exp7_server_scale_bottleneck=14
exp7_method_list=("farreach" "netcache" "nocache")
exp7_dynamic_rule_list=(hotin hotout random stable)
exp7_output_path="${EVALUATION_OUTPUT_PREFIX}/exp7/${roundnumber}"

### Create output directory
mkdir -p ${exp7_output_path}

for exp7_method in ${exp7_method_list[@]}; do
  ### Preparation
  sed -i "/^DIRNAME=/s/=.*/=\"${exp7_method}\"/" ${CLIENT_ROOTPATH}/scripts/common.sh
  cd ${CLIENT_ROOTPATH}
  bash scripts/remote/sync_file.sh scripts common.sh

  for exp7_rule in ${exp7_dynamic_rule_list[@]}; do
    echo "[exp7][${exp7_method}][${exp7_rule}]"

    echo "[exp7][${exp7_method}][${exp7_rule}] run rulemap with ${exp7_rule}"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp7_method}; bash localscripts/stopswitchtestbed.sh"
    
    echo "[exp7][${exp7_method}][${exp7_rule}] update ${exp7_method} config"
    cp ${CLIENT_ROOTPATH}/${exp7_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp7_workload}"/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^workload_mode=/s/=.*/=1/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^dynamic_ruleprefix=/s/=.*/=${exp7_rule}/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp7_server_scale}"/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp7_server_scale_for_rotation}"/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp7_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
	sed -i "s/^server_logical_idxes=TODO0/server_logical_idxes=0/g" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini
	sed -i "s/^server_logical_idxes=TODO1/server_logical_idxes=1/g" ${CLIENT_ROOTPATH}/${exp7_method}/config.ini

    cd ${CLIENT_ROOTPATH}
    echo "[exp7][${exp7_method}][${exp7_rule}] prepare config.ini" 
    bash scripts/remote/sync_file.sh ${exp7_method} config.ini

    echo "[exp7][${exp7_method}][${exp7_rule}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp7_method}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp7_method}; bash localscripts/launchswitchostestbed.sh"

    sleep 20s


    ### Evaluation
    echo "[exp7][${exp7_method}][${exp7_rule}] test server rotation" 
    bash scripts/remote/test_dynamic.sh

    echo "[exp7][${exp7_method}][${exp7_rule}] sync json file and calculate"
    bash scripts/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/synthetic-statistics/${exp7_method}-${exp7_rule}-client0.out  ${exp7_output_path}/synthetic-${exp7_method}-${exp7_rule}-client0.out
    cp ${CLIENT_ROOTPATH}/benchmark/output/synthetic-statistics/${exp7_method}-${exp7_rule}-client1.out  ${exp7_output_path}/synthetic-${exp7_method}-${exp7_rule}-client1.out
    echo "[exp7][${exp7_method}] stop switchos"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp7_method}; bash localscripts/stopswitchtestbed.sh"
  done
done

