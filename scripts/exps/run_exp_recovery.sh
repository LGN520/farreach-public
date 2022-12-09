#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_recovery

source scripts/global.sh
if [ $# -ne 2 ]; then
	echo "Usage: bash scripts/exps/run_exp_recovery.sh <workloadmode> <recoveryonly>"
	exit
fi
exp9_workloadmode=$1
exp9_recoveryonly=$2

if [ ${exp9_recoveryonly} -ne 1 ] && [ ${exp9_recoveryonly} -ne 0 ]; then
	echo "recoveryonly should be either 1 or 0"
	exit
fi

exp9_server_scale="16"
exp9_server_scale_bottleneck="14"
exp9_round_list=("0" "1" "2" "3" "4" "5") # do one extra round 0 to wait for database to finish flush and compaction
exp9_cachesize_list=("100" "1000" "10000")


ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/stopswitchtestbed.sh"

### Recovery
for exp9_cachesize in ${exp9_cachesize_list[@]}; do
  echo "[exp9][${exp9_cachesize}] Recovery with cache size of ${exp9_cachesize}"
  cp ${CLIENT_ROOTPATH}/farreach/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^workload_name=/s/=.*/=farreach/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^workload_mode=/s/=.*/="${exp9_workloadmode}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^server_total_logical_num=/s/=.*/="${exp9_server_scale}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp9_server_scale}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp9_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^controller_snapshot_period=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^switch_kv_bucket_num=/s/=.*/="${exp9_cachesize}"/" ${CLIENT_ROOTPATH}/farreach/config.ini

  bash scripts/remote/sync_file.sh farreach config.ini

  cd ${CLIENT_ROOTPATH}
  echo "[exp9][${exp9_cachesize}] Prepare server rotation" 
  bash scripts/remote/prepare_server_rotation.sh

  if [ ${exp9_recoveryonly} -eq 0 ]; then
    echo "[exp9][${exp9_cachesize}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/launchswitchostestbed.sh"

    sleep 20s

    echo "[exp9][${exp9_cachesize}] test server rotation" 
    bash scripts/remote/test_server_rotation.sh

    echo "[exp9][${exp9_cachesize}] stop server rotation"
    bash scripts/remote/stop_server_rotation.sh

    echo "[exp9][${exp9_cachesize}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/stopswitchtestbed.sh"
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

      scp ${USER}@${SERVER0}:/tmp/farreach/controller.snapshot* ${exp9_output_path}/${exp9_cachesize}
      scp ${USER}@${SERVER0}:/tmp/farreach/*maxseq* ${exp9_output_path}/${exp9_cachesize}
      scp ${USER}@${SERVER1}:/tmp/farreach/*maxseq* ${exp9_output_path}/${exp9_cachesize}
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

      scp ${exp9_output_path}/${exp9_cachesize}/controller.snapshot* ${USER}@${SERVER0}:/tmp/farreach/
      scp ${exp9_output_path}/${exp9_cachesize}/*maxseq* ${USER}@${SERVER0}:/tmp/farreach/
      scp ${exp9_output_path}/${exp9_cachesize}/*maxseq* ${USER}@${SERVER1}:/tmp/farreach/
    fi

    echo "[exp9][${exp9_roundnumber}][${exp9_cachesize}] Get recovery time"
    bash scripts/remote/test_recovery_time.sh > tmp_test_recovery_time.out 2>&1

    echo "[exp9][${exp9_roundnumber}][${exp9_cachesize}] Backup statistics files to ${exp9_output_path}/${exp9_cachesize}"
    sleep 10s
    cp tmp_test_recovery_time.out ${exp9_output_path}/${exp9_cachesize}
    scp -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH}:${SWITCH_ROOTPATH}/farreach/tmp_launchswitchostestbed.out ${exp9_output_path}/${exp9_cachesize}
    scp -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH}:${SWITCH_ROOTPATH}/farreach/tmp_switchos.out ${exp9_output_path}/${exp9_cachesize}
    scp ${USER}@${SERVER0}:${CLIENT_ROOTPATH}/farreach/tmp_server.out ${exp9_output_path}/${exp9_cachesize}/tmp_server_0.out
    scp ${USER}@${SERVER1}:${CLIENT_ROOTPATH}/farreach/tmp_server.out ${exp9_output_path}/${exp9_cachesize}/tmp_server_1.out

    python scripts/local/calculate_recovery_time_helper.py ${exp9_output_path}/${exp9_cachesize} ${exp9_server_scale}
  done
done
