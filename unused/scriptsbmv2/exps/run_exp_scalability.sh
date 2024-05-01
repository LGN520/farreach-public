set -x
#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_scalability

source scriptsbmv2/global.sh
if [ $# -ne 1 ]; then
	echo "Usage: bash scriptsbmv2/exps/run_exp_scalability.sh <roundnumber>"
	exit
fi
roundnumber=$1

exp3_workload="workloada"
exp3_method_list=("farreach" "netcache" "nocache")
exp3_scalability_list=("32" "64" "128")
exp3_scalability_bottleneck_list=("29" "59" "118")
exp3_output_path="${EVALUATION_OUTPUT_PREFIX}/exp3/${roundnumber}"

### Create json backup directory
mkdir -p ${exp3_output_path}

for exp3_method in ${exp3_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp3_method}\"/" ${CLIENT_ROOTPATH}/scriptsbmv2/common.sh
  scalability_counter=0
  for exp3_scalability in ${exp3_scalability_list[@]}; do
    echo "[exp3][${exp3_method}][${exp3_scalability}]"

    ### Preparation
    echo "[exp3][${exp3_method}][${exp3_scalability}] run workload with ${exp3_scalability}" servers
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp3_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
    
    echo "[exp3][${exp3_method}][${exp3_scalability}] update ${exp3_method} config with workloada"
    cp ${CLIENT_ROOTPATH}/${exp3_method}/configs/config.ini.bmv2 ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp3_workload}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp3_scalability}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp3_scalability}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp3_scalability_bottleneck_list[${scalability_counter}]}"/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp3_method}/config.ini


    cd ${CLIENT_ROOTPATH}
    echo "[exp3][${exp3_method}][${exp3_scalability}] prepare server rotation" 
    bash scriptsbmv2/remote/prepare_server_rotation.sh

    echo "[exp3][${exp3_method}][${exp3_scalability}] start switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp3_method}/bmv2; nohup python network.py &"
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp3_method}; bash localscriptsbmv2/launchswitchostestbed.sh"

    sleep 20s

    ### Evaluation
    echo "[exp3][${exp3_method}][${exp3_scalability}] test server rotation" 
    bash scriptsbmv2/remote/test_server_rotation.sh

    echo "[exp3][${exp3_method}][${exp3_scalability}] stop server rotation"
    bash scriptsbmv2/remote/stop_server_rotation.sh

    echo "[exp3][${exp3_method}][${exp3_scalability}] sync json file and calculate"
    bash scriptsbmv2/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static${exp3_scalability}-client0.out  ${exp3_output_path}/${exp3_workload}-${exp3_method}-static${exp3_scalability}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static${exp3_scalability}-client1.out  ${exp3_output_path}/${exp3_workload}-${exp3_method}-static${exp3_scalability}-client1.out 
    echo "[exp3][${exp3_method}] stop switchos" 
    ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp3_method}; bash localscriptsbmv2/stopswitchtestbed.sh"
    scalability_counter=$((++scalability_counter))
  done

  cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static16-client0.out  ${exp3_output_path}/${exp3_workload}-${exp3_method}-static16-client0.out 
  cp ${CLIENT_ROOTPATH}/benchmark/output/${exp3_workload}-statistics/${exp3_method}-static16-client1.out  ${exp3_output_path}/${exp3_workload}-${exp3_method}-static16-client1.out 
done
