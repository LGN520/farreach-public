#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_recovery_snapshot

source scripts/global.sh
if [ $# -ne 2 ]; then
	echo "Usage: bash scripts/exps/run_exp_recovery_snapshot.sh <workloadmode> <recoveryonly>"
	exit
fi
exp10_workloadmode=$1
exp10_recoveryonly=$2

if [ ${exp10_recoveryonly} -ne 1 ] && [ ${exp10_recoveryonly} -ne 0 ]; then
	echo "recoveryonly should be either 1 or 0"
	exit
fi

exp10_server_scale="16"
exp10_server_scale_bottleneck="14"
exp10_round_list=("0" "1" "2" "3" "4" "5") # do one extra round 0 to wait for database to finish flush and compaction
exp10_snapshot_list=("0" "2500" "5000" "7500" "10000")


ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/stopswitchtestbed.sh"

### Recovery
for exp10_snapshot in ${exp10_snapshot_list[@]}; do
  echo "[exp9][${exp10_snapshot}] Recovery with snapshot interval of ${exp10_snapshot} ms"
  cp ${CLIENT_ROOTPATH}/farreach/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^workload_name=/s/=.*/=synthetic/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^workload_mode=/s/=.*/="${exp10_workloadmode}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^server_total_logical_num=/s/=.*/="${exp10_server_scale}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp10_server_scale}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp10_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^controller_snapshot_period=/s/=.*/="${exp10_snapshot}"/" ${CLIENT_ROOTPATH}/farreach/config.ini
  sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/farreach/config.ini

  bash scripts/remote/sync_file.sh farreach config.ini

  cd ${CLIENT_ROOTPATH}
  echo "[exp9][${exp10_snapshot}] Prepare server rotation" 
  bash scripts/remote/prepare_server_rotation.sh

  if [ ${exp10_recoveryonly} -eq 0 ]; then
    echo "[exp9][${exp10_snapshot}] start switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach/tofino; nohup bash start_switch.sh > tmp_start_switch.out 2>&1 &"
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/launchswitchostestbed.sh"

    sleep 20s

    echo "[exp9][${exp10_snapshot}] test server rotation" 
    bash scripts/remote/test_server_rotation.sh

    echo "[exp9][${exp10_snapshot}] stop server rotation"
    bash scripts/remote/stop_server_rotation.sh

    echo "[exp9][${exp10_snapshot}] stop switchos" 
    ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/farreach; bash localscripts/stopswitchtestbed.sh"
  fi

  for exp10_roundnumber in ${exp10_round_list[@]}; do
    exp10_output_path="${EVALUATION_OUTPUT_PREFIX}/exp9/${exp10_roundnumber}"
    mkdir -p ${exp10_output_path}/${exp10_snapshot}

    if [ ${exp10_recoveryonly} -eq 0 ]; then
      if [ ${exp10_workloadmode} -eq 0 ]; then
        cp benchmark/output/upstreambackups/static${exp10_server_scale}*client0.out ${exp10_output_path}/${exp10_snapshot}
        scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/static${exp10_server_scale}*client1.out ${exp10_output_path}/${exp10_snapshot}
      elif [ ${exp10_workloadmode} -eq 1 ]; then
        cp benchmark/output/upstreambackups/dynamic-client0.out ${exp10_output_path}/${exp10_snapshot}
        scp ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/dynamic-client1.out ${exp10_output_path}/${exp10_snapshot}
      else
        echo "[ERROR] invalid workload mode: ${exp10_workloadmode}"
        exit
      fi

      scp ${USER}@${SERVER0}:/tmp/farreach/controller.snapshot* ${exp10_output_path}/${exp10_snapshot}
      scp ${USER}@${SERVER0}:/tmp/farreach/*maxseq* ${exp10_output_path}/${exp10_snapshot}
      scp ${USER}@${SERVER1}:/tmp/farreach/*maxseq* ${exp10_output_path}/${exp10_snapshot}
    else
      if [ ${exp10_workloadmode} -eq 0 ]; then
        cp ${exp10_output_path}/${exp10_snapshot}/static${exp10_server_scale}*client0.out benchmark/output/upstreambackups/
        scp ${exp10_output_path}/${exp10_snapshot}/static${exp10_server_scale}*client1.out  ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/
      elif [ ${exp10_workloadmode} -eq 1 ]; then
        cp ${exp10_output_path}/${exp10_snapshot}/dynamic-client0.out benchmark/output/upstreambackups/
        scp ${exp10_output_path}/${exp10_snapshot}/dynamic-client1.out ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/benchmark/output/upstreambackups/
      else
        echo "[ERROR] invalid workload mode: ${exp10_workloadmode}"
        exit
      fi

      scp ${exp10_output_path}/${exp10_snapshot}/controller.snapshot* ${USER}@${SERVER0}:/tmp/farreach/
      scp ${exp10_output_path}/${exp10_snapshot}/*maxseq* ${USER}@${SERVER0}:/tmp/farreach/
      scp ${exp10_output_path}/${exp10_snapshot}/*maxseq* ${USER}@${SERVER1}:/tmp/farreach/
    fi

    echo "[exp9][${exp10_roundnumber}][${exp10_snapshot}] Get recovery time"
    bash scripts/remote/test_recovery_time.sh > tmp_test_recovery_time.out 2>&1

    echo "[exp9][${exp10_roundnumber}][${exp10_snapshot}] Backup statistics files to ${exp10_output_path}/${exp10_snapshot}"
    sleep 10s
    cp tmp_test_recovery_time.out ${exp10_output_path}/${exp10_snapshot}
    scp -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH}:${SWITCH_ROOTPATH}/farreach/tmp_launchswitchostestbed.out ${exp10_output_path}/${exp10_snapshot}
    scp -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH}:${SWITCH_ROOTPATH}/farreach/tmp_switchos.out ${exp10_output_path}/${exp10_snapshot}
    scp ${USER}@${SERVER0}:${CLIENT_ROOTPATH}/farreach/tmp_server.out ${exp10_output_path}/${exp10_snapshot}/tmp_server_0.out
    scp ${USER}@${SERVER1}:${CLIENT_ROOTPATH}/farreach/tmp_server.out ${exp10_output_path}/${exp10_snapshot}/tmp_server_1.out

    python scripts/local/calculate_recovery_time_helper.py ${exp10_output_path}/${exp10_snapshot} ${exp10_server_scale}
  done
done
