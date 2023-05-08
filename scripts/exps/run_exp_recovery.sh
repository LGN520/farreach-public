#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_recovery

source scripts/global.sh
if [ $# -ne 0 ]; then
	echo "Usage: bash scripts/exps/run_exp_recovery.sh"
	exit
fi
exp9_workloadmode=0
exp9_recoveryonly=0

if [ ${exp9_workloadmode} -eq 1 ]
then
	echo "[ERROR] we have NOT checked dynamic pattern in this script, please use workloadmode=0!"
	exit
fi

if [ ${exp9_recoveryonly} -ne 1 ] && [ ${exp9_recoveryonly} -ne 0 ]; then
	echo "recoveryonly should be either 1 or 0"
	exit
fi

# NOTE: do NOT change exp9_method, which MUST be farreach!!!
exp9_method="farreach"
if [ "x${exp9_method}" != "xfarreach" ]
then
	echo "[ERROR] you can only use this script for FarReach!"
	exit
fi

exp9_server_scale="16"
if [ ${exp9_workloadmode} -eq 1 ]
then
	exp9_server_scale="2"
fi

# NOTE: we ONLY support synthetic workload here!!!
exp9_server_scale_for_rotation="16"
exp9_server_scale_bottleneck="14"
exp9_round_list=("0" "1" "2" "3" "4" "5") # do one extra round 0 to wait for database to finish flush and compaction
exp9_cachesize_list=("100" "1000" "10000")

# NOTE: ONLY for dynamic pattern; you can set it as hotin / hotout / random
exp9_rule="hotin"

### Prepare for DIRNAME
sed -i "/^DIRNAME=/s/=.*/=\"${exp9_method}\"/" ${CLIENT_ROOTPATH}/scripts/common.sh
cd ${CLIENT_ROOTPATH}
bash scripts/remote/sync_file.sh scripts common.sh

ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp9_method}; bash localscripts/stopswitchtestbed.sh"

### Recovery
for exp9_cachesize in ${exp9_cachesize_list[@]}; do
  echo "[exp9][${exp9_cachesize}] Recovery with cache size of ${exp9_cachesize}"
  cp ${CLIENT_ROOTPATH}/${exp9_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  sed -i "/^workload_name=/s/=.*/=synthetic/" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  sed -i "/^workload_mode=/s/=.*/="${exp9_workloadmode}"/" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  sed -i "/^server_total_logical_num=/s/=.*/="${exp9_server_scale}"/" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp9_server_scale_for_rotation}"/" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp9_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  sed -i "/^switch_kv_bucket_num=/s/=.*/="${exp9_cachesize}"/" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  if [ ${exp9_workloadmode} -eq 1 ]
  then
    sed -i "s/^server_logical_idxes=TODO0/server_logical_idxes=0/g" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
    sed -i "s/^server_logical_idxes=TODO1/server_logical_idxes=1/g" ${CLIENT_ROOTPATH}/${exp9_method}/config.ini
  fi

  bash scripts/remote/sync_file.sh ${exp9_method} config.ini

  if [ ${exp9_workloadmode} -ne 1 ]
  then
    cd ${CLIENT_ROOTPATH}
    echo "[exp9][${exp9_cachesize}] Prepare server rotation" 
    bash scripts/remote/prepare_server_rotation.sh
  fi

  if [ ${exp9_recoveryonly} -eq 0 ]; then
    echo "[exp9][${exp9_cachesize}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp9_method}/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp9_method}; bash localscripts/launchswitchostestbed.sh"

    sleep 20s

    if [ ${exp9_workloadmode} -eq 1 ]
	then
      echo "[exp9][${exp9_rule}][10000ms] test dynamic workload pattern without server rotation" 
      bash scripts/remote/test_dynamic.sh
    else
      echo "[exp9][${exp9_cachesize}] test server rotation" 
      bash scripts/remote/test_server_rotation.sh

      echo "[exp9][${exp9_cachesize}] stop server rotation"
      bash scripts/remote/stop_server_rotation.sh
	fi

    echo "[exp9][${exp9_cachesize}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${exp9_method}; bash localscripts/stopswitchtestbed.sh"
  fi

  for exp9_roundnumber in ${exp9_round_list[@]}; do
    exp9_output_path="${EVALUATION_OUTPUT_PREFIX}/exp9/${exp9_roundnumber}"
    mkdir -p ${exp9_output_path}/${exp9_cachesize}

    if [ ${exp9_recoveryonly} -eq 0 ]; then
      if [ ${exp9_workloadmode} -eq 0 ]; then
        cp benchmark/output/upstreambackups/static${exp9_server_scale}*client0.out ${exp9_output_path}/${exp9_cachesize}
        scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/static${exp9_server_scale}*client1.out ${exp9_output_path}/${exp9_cachesize}
      elif [ ${exp9_workloadmode} -eq 1 ]; then
        cp benchmark/output/upstreambackups/dynamic-client0.out ${exp9_output_path}/${exp9_cachesize}
        scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/dynamic-client1.out ${exp9_output_path}/${exp9_cachesize}
      else
        echo "[ERROR] invalid workload mode: ${exp9_workloadmode}"
        exit
      fi

      scp ${USER}@${SERVER0}:/tmp/${exp9_method}/controller.snapshot* ${exp9_output_path}/${exp9_cachesize}
      scp ${USER}@${SERVER0}:/tmp/${exp9_method}/*maxseq* ${exp9_output_path}/${exp9_cachesize}
      scp ${USER}@${SERVER1}:/tmp/${exp9_method}/*maxseq* ${exp9_output_path}/${exp9_cachesize}
    else
      if [ ${exp9_workloadmode} -eq 0 ]; then
        cp ${exp9_output_path}/${exp9_cachesize}/static${exp9_server_scale}*client0.out benchmark/output/upstreambackups/
        scp ${exp9_output_path}/${exp9_cachesize}/static${exp9_server_scale}*client1.out  ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/
      elif [ ${exp9_workloadmode} -eq 1 ]; then
        cp ${exp9_output_path}/${exp9_cachesize}/dynamic-client0.out benchmark/output/upstreambackups/
        scp ${exp9_output_path}/${exp9_cachesize}/dynamic-client1.out ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/
      else
        echo "[ERROR] invalid workload mode: ${exp9_workloadmode}"
        exit
      fi

      scp ${exp9_output_path}/${exp9_cachesize}/controller.snapshot* ${USER}@${SERVER0}:/tmp/${exp9_method}/
      scp ${exp9_output_path}/${exp9_cachesize}/*maxseq* ${USER}@${SERVER0}:/tmp/${exp9_method}/
      scp ${exp9_output_path}/${exp9_cachesize}/*maxseq* ${USER}@${SERVER1}:/tmp/${exp9_method}/
    fi

    echo "[exp9][${exp9_roundnumber}][${exp9_cachesize}] Get recovery time"
    bash scripts/remote/test_recovery_time.sh > tmp_test_recovery_time.out 2>&1

    echo "[exp9][${exp9_roundnumber}][${exp9_cachesize}] Backup statistics files to ${exp9_output_path}/${exp9_cachesize}"
    sleep 10s
    cp tmp_test_recovery_time.out ${exp9_output_path}/${exp9_cachesize}
    scp -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH}:${SWITCH_ROOTPATH}/${exp9_method}/tmp_launchswitchostestbed.out ${exp9_output_path}/${exp9_cachesize}
    scp -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH}:${SWITCH_ROOTPATH}/${exp9_method}/tmp_switchos.out ${exp9_output_path}/${exp9_cachesize}
    scp ${USER}@${SERVER0}:${CLIENT_ROOTPATH}/${exp9_method}/tmp_server.out ${exp9_output_path}/${exp9_cachesize}/tmp_server_0.out
    scp ${USER}@${SERVER1}:${CLIENT_ROOTPATH}/${exp9_method}/tmp_server.out ${exp9_output_path}/${exp9_cachesize}/tmp_server_1.out

    python scripts/local/calculate_recovery_time_helper.py ${exp9_output_path}/${exp9_cachesize} ${exp9_server_scale}
  done
done
