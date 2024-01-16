# set -x
#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_ycsb

source scriptsdist/global.sh

if [ $# -ne 1 ]; then
	echo "Usage: bash scriptsdist/exps/run_exp1.sh <roundnumber>"
	exit
fi
roundnumber=$1
# "netcache" "nocache" "workloadb" "workloadc" "workloadd" "workloadf" "workload-load"
exp1_method_list=("farreach" )
exp1_core_workload_list=("workloada")
exp1_server_scale=16
exp1_output_path="${EVALUATION_OUTPUT_PREFIX}/exp1/${roundnumber}"

### Create json backup directory
mkdir -p ${exp1_output_path}/

for exp1_method in ${exp1_method_list[@]}; do
  sed -i "/^DIRNAME=/s/=.*/=\"${exp1_method}\"/" ${CLIENT_ROOTPATH}/scriptsdist/common.sh
  cd ${CLIENT_ROOTPATH}
  # bash scriptsdist/remote/sync_file.sh scripts common.sh

  for exp1_workload in ${exp1_core_workload_list[@]}; do
    echo "[exp1][${exp1_method}][${exp1_workload}]"

    ### Preparation
    echo "[exp1][${exp1_method}][${exp1_workload}] run workload with $exp1_workload servers"
    cd ${SWITCH_ROOTPATH}/${exp1_method} 
    bash localscriptsdist/stopswitchtestbed.sh
    
    echo "[exp1][${exp1_method}][${exp1_workload}] update ${exp1_method} config with ${exp1_workload}"
    cp ${CLIENT_ROOTPATH}/${exp1_method}/configs/config.ini.static.setup.bmv2 ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^workload_name=/s/=.*/="${exp1_workload}"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^server_total_logical_num=/s/=.*/="${exp1_server_scale}"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp1_server_scale}"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    if [ "x${exp1_workload}" == "xworkload-load" ]; then
      sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="1"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    elif [ "x${exp1_workload}" == "xworkloadd" ]; then
      sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="3"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    else
      sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="2"/" ${CLIENT_ROOTPATH}/${exp1_method}/config.ini
    fi

    cd ${CLIENT_ROOTPATH}
    echo "[exp1][${exp1_method}][${exp1_workload}] prepare server rotation" 
    bash scriptsdist/remote/prepare_server_rotation.sh
    # ??
  
    echo "[exp1][${exp1_method}][${exp1_workload}] start switchos" 
    cd ${SWITCH_ROOTPATH}/${exp1_method}/bmv2; 
    nohup python network.py &
    sleep 10s
    # ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "
    cd ${SWITCH_ROOTPATH}/${exp1_method}; 
    bash localscriptsdist/launchswitchostestbed.sh

    ### Evaluation
    cd ${SWITCH_ROOTPATH}
    echo "[exp1][${exp1_method}][${exp1_workload}] test server rotation" 
    bash scriptsdist/remote/test_server_rotation.sh

    echo "[exp1][${exp1_method}][${exp1_workload}] stop server rotation"
    bash scriptsdist/remote/stop_server_rotation.sh

    echo "[exp1][${exp1_method}][${exp1_workload}] sync json file and calculate"
    bash scriptsdist/remote/calculate_statistics.sh 0


    ### Cleanup
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp1_workload}-statistics/${exp1_method}-static${exp1_server_scale}-client0.out  ${exp1_output_path}/${exp1_workload}-${exp1_method}-static${exp1_server_scale}-client0.out 
    cp ${CLIENT_ROOTPATH}/benchmark/output/${exp1_workload}-statistics/${exp1_method}-static${exp1_server_scale}-client1.out  ${exp1_output_path}/${exp1_workload}-${exp1_method}-static${exp1_server_scale}-client1.out 
    echo "[exp1][${exp1_method}][${exp1_workload}] stop switchos" 
    cd ${SWITCH_ROOTPATH}/${exp1_method}; 
    bash localscriptsdist/stopswitchtestbed.sh
  done
done


