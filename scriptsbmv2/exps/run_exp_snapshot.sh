#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_snapshot

source scripts/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scripts/exps/run_exp_snapshot.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp8_server_scale=2
exp8_server_scale_for_rotation=16
exp8_server_scale_bottleneck=14
# NOTE: do NOT change exp8_method, which MUST be farreach!!!
exp8_method="farreach"
exp8_workload="synthetic"
exp8_dynamic_rule_list=(hotin hotout random)
exp8_snapshot_list=("0" "2500" "5000" "7500" "10000")
exp8_output_path="${EVALUATION_OUTPUT_PREFIX}/exp8/${roundnumber}"

if [ "x${exp8_method}" != "xfarreach" ]
then
	echo "[ERROR] you can only use this script for FarReach!"
	exit
fi

### Create json backup directory
mkdir -p ${exp8_output_path}

### Prepare for DIRNAME
sed -i "/^DIRNAME=/s/=.*/=\"${exp8_method}\"/" ${CLIENT_ROOTPATH}/scripts/common.sh
cd ${CLIENT_ROOTPATH}
bash scripts/remote/sync_file.sh scripts common.sh

for exp8_rule in ${exp8_dynamic_rule_list[@]}; do
  for exp8_snapshot in ${exp8_snapshot_list[@]}; do
    ### Preparation
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] run rulemap with ${exp8_rule}"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp8_method}; bash localscripts/stopswitchtestbed.sh"
    
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] update ${exp8_method} config with snapshot"
    cp ${CLIENT_ROOTPATH}/${exp8_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp8_workload}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^workload_mode=/s/=.*/=1/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^dynamic_ruleprefix=/s/=.*/=${exp8_rule}/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp8_server_scale}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp8_server_scale_for_rotation}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp8_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/="${exp8_snapshot}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
	sed -i "s/^server_logical_idxes=TODO0/server_logical_idxes=0/g" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
	sed -i "s/^server_logical_idxes=TODO1/server_logical_idxes=1/g" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini

    cd ${CLIENT_ROOTPATH}
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] prepare config.ini" 
    bash scripts/remote/sync_file.sh ${exp8_method} config.ini

    echo "[exp8][${exp8_rule}][${exp8_snapshot}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp8_method}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp8_method}; bash localscripts/launchswitchostestbed.sh"

    sleep 20s


    ### Evaluation
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] test dynamic workload pattern without server rotation" 
    bash scripts/remote/test_dynamic.sh

    echo "[exp8][${exp8_rule}][${exp8_snapshot}] sync json file and calculate"
    bash scripts/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/synthetic-statistics/${exp8_method}-${exp8_rule}-client0.out  ${exp8_output_path}/${exp8_snapshot}-${exp8_method}-${exp8_rule}-client0.out
    cp ${CLIENT_ROOTPATH}/benchmark/output/synthetic-statistics/${exp8_method}-${exp8_rule}-client1.out  ${exp8_output_path}/${exp8_snapshot}-${exp8_method}-${exp8_rule}-client1.out
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp8_method}; bash localscripts/stopswitchtestbed.sh"

    scp ${USER}@${SERVER0}:${SERVER_ROOTPATH}/${exp8_method}/tmp_controller_bwcost.out ${exp8_output_path}/${exp8_snapshot}_${exp8_rule}_tmp_controller_bwcost.out

	cd scripts/local/
	python calculate_bwcost_helper.py ${exp8_output_path}/${exp8_snapshot}_${exp8_rule}_tmp_controller_bwcost.out
	cd ../../
  done
done

