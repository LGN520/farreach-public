#!/bin/bash
# run this scripts on dl11 (main client)
# exp_snapshot

source scripts/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scripts/exps/run_exp_snapshot.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp8_dynamic_rule_list=(hotin hotout random)
exp8_snapshot_list=("0" "2500" "5000" "7500" "10000")

### Create json backup directory
mkdir -p ${EVALUATION_OUTPUT_PREFIX}/exp8/${roundnumber}/

for exp8_rule in ${exp8_dynamic_rule_list[@]}; do
  for exp8_snapshot in ${exp8_snapshot_list[@]}; do
    ### Preparation
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] run rulemap with ${exp8_rule}"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/stopswitchtestbed.sh"
    
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] update farreach config with snapshot"
    cp ${CLIENT_ROOTPATH}/${exp8_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp8_workload}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^workload_mode=/s/=.*/=1/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^dynamic_ruleprefix=/s/=.*/=${exp8_rule}/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp8_server_scale}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp8_server_scale}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp8_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/="${exp8_snapshot}"/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp8_method}/config.ini

    cd ${CLIENT_ROOTPATH}
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] prepare config.ini" 
    bash scripts/remote/sync_file.sh farreach config.ini

    echo "[exp8][${exp8_rule}][${exp8_snapshot}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/farreach/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/launchswitchostestbed.sh"

    sleep 20s


    ### Evaluation
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] test server rotation" 
    bash scripts/remote/test_dynamic.sh

    echo "[exp8][${exp8_rule}][${exp8_snapshot}] sync json file and calculate"
    bash scripts/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/synthetic-statistics/farreach-${exp8_rule}-client0.out  ${EVALUATION_OUTPUT_PREFIX}/exp8/${roundnumber}/${exp8_snapshot}-farreach-${exp8_rule}-client0.out
    cp ${CLIENT_ROOTPATH}/benchmark/output/synthetic-statistics/farreach-${exp8_rule}-client1.out  ${EVALUATION_OUTPUT_PREFIX}/exp8/${roundnumber}/${exp8_snapshot}-farreach-${exp8_rule}-client1.out
    echo "[exp8][${exp8_rule}][${exp8_snapshot}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/stopswitchtestbed.sh"

    scp ${USER}@dl16:${SERVER_ROOTPATH}/farreach/tmp_controller_bwcost.out ${EVALUATION_OUTPUT_PREFIX}/exp8/${roundnumber}/${exp8_snapshot}_${exp8_rule}_tmp_controller_bwcost.out
  done
done

