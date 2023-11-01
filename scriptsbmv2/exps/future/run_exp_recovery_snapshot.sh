set -x
#!/bin/bash
# run this scripts on ${MAIN_CLIENT} (main client)
# exp_recovery_snapshot

#
# NOTE: this script has NOT been checked (left in the future)!!!
#

source scriptsbmv2/global.sh
if [ $# -ne 2 ]; then
	echo "Usage: bash scriptsbmv2/exps/run_exp_recovery_snapshot.sh <workloadmode> <recoveryonly>"
	exit
fi
exp10_workloadmode=$1
exp10_recoveryonly=$2

if [ ${exp10_workloadmode} -eq 1 ]
then
	echo "[ERROR] we have NOT checked dynamic pattern in this script, please use workloadmode=0!"
	exit
fi

if [ ${exp10_recoveryonly} -ne 1 ] && [ ${exp10_recoveryonly} -ne 0 ]; then
	echo "recoveryonly should be either 1 or 0"
	exit
fi

# NOTE: do NOT change exp10_method, which MUST be farreach!!!
exp10_method="farreach"
if [ "x${exp10_method}" != "xfarreach" ]
then
	echo "[ERROR] you can only use this script for FarReach!"
	exit
fi

exp10_server_scale=16
if [ ${exp10_workloadmode} -eq 1 ]
then
	exp10_server_scale="2"
fi

# NOTE: we ONLY support synthetic workload here!!!
exp10_server_scale_for_rotation="16"
exp10_server_scale_bottleneck=4
exp10_round_list=("0" "1" "2" "3" "4" "5") # do one extra round 0 to wait for database to finish flush and compaction
exp10_snapshot_list=("0" "2500" "5000" "7500" "10000")

# NOTE: ONLY for dynamic pattern; you can set it as hotin / hotout / random
exp10_rule="hotin"

### Prepare for DIRNAME
sed -i "/^DIRNAME=/s/=.*/=\"${exp10_method}\"/" ${CLIENT_ROOTPATH}/scriptsbmv2/common.sh
cd ${CLIENT_ROOTPATH}
bash scriptsbmv2/remote/sync_file.sh scripts common.sh

cd ${SWITCH_ROOTPATH}/${exp10_method}; 
bash localscriptsbmv2/stopswitchtestbed.sh
cd ${SWITCH_ROOTPATH}
### Recovery
for exp10_snapshot in ${exp10_snapshot_list[@]}; do
  echo "[exp10][${exp10_snapshot}] Recovery with snapshot interval of ${exp10_snapshot} ms"
  cp ${CLIENT_ROOTPATH}/${exp10_method}/configs/config.ini.static.setup ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  sed -i "/^workload_name=/s/=.*/=synthetic/" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  sed -i "/^workload_mode=/s/=.*/="${exp10_workloadmode}"/" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  sed -i "/^server_total_logical_num=/s/=.*/="${exp10_server_scale}"/" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  sed -i "/^server_total_logical_num_for_rotation=/s/=.*/="${exp10_server_scale_for_rotation}"/" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  sed -i "/^bottleneck_serveridx_for_rotation=/s/=.*/="${exp10_server_scale_bottleneck}"/" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  sed -i "/^controller_snapshot_period=/s/=.*/="${exp10_snapshot}"/" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  sed -i "/^switch_kv_bucket_num=/s/=.*/=10000/" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  if [ ${exp10_workloadmode} -eq 1 ]
  then
    sed -i "s/^server_logical_idxes=TODO0/server_logical_idxes=0/g" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
    sed -i "s/^server_logical_idxes=TODO1/server_logical_idxes=1/g" ${CLIENT_ROOTPATH}/${exp10_method}/config.ini
  fi

  # bash scriptsbmv2/remote/sync_file.sh ${exp10_method} config.ini

  if [ ${exp10_workloadmode} -ne 1 ]
  then
    cd ${CLIENT_ROOTPATH}
    echo "[exp10][${exp10_snapshot}] Prepare server rotation" 
    bash scriptsbmv2/remote/prepare_server_rotation.sh
  fi

  if [ ${exp10_recoveryonly} -eq 0 ]; then
    echo "[exp10][${exp10_snapshot}] start switchos" 
    
    cd ${SWITCH_ROOTPATH}/${exp10_method}/bmv2; python network.py > tmp_start_switch.out 2>&1 &
    sleep 10s
    cd ${SWITCH_ROOTPATH}/${exp10_method}; bash localscriptsbmv2/launchswitchostestbed.sh
    cd ${SWITCH_ROOTPATH}
    sleep 20s

    if [ ${exp10_workloadmode} -eq 1 ]
	then
      echo "[exp10][${exp10_rule}][10000ms] test dynamic workload pattern without server rotation" 
      bash scriptsbmv2/remote/test_dynamic.sh
    else
      echo "[exp10][${exp10_snapshot}] test server rotation" 
      bash scriptsbmv2/remote/test_server_rotation.sh

      echo "[exp10][${exp10_snapshot}] stop server rotation"
      bash scriptsbmv2/remote/stop_server_rotation.sh
    fi

    echo "[exp10][${exp10_snapshot}] stop switchos" 
    cd ${SWITCH_ROOTPATH}/${exp10_method}; 
    bash localscriptsbmv2/stopswitchtestbed.sh
  fi

  for exp10_roundnumber in ${exp10_round_list[@]}; do
    exp10_output_path="${EVALUATION_OUTPUT_PREFIX}/exp10/${exp10_roundnumber}"
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

      cp /tmp/${exp10_method}/controller.snapshot* ${exp10_output_path}/${exp10_snapshot}
      cp /tmp/${exp10_method}/*maxseq* ${exp10_output_path}/${exp10_snapshot}
      # cp /tmp/${exp10_method}/*maxseq* ${exp10_output_path}/${exp10_snapshot}
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

      cp ${exp10_output_path}/${exp10_snapshot}/controller.snapshot* /tmp/${exp10_method}/
      cp ${exp10_output_path}/${exp10_snapshot}/*maxseq* /tmp/${exp10_method}/
      # cp ${exp10_output_path}/${exp10_snapshot}/*maxseq* ${USER}@${SERVER1}:/tmp/${exp10_method}/
    fi

    echo "[exp10][${exp10_roundnumber}][${exp10_snapshot}] Get recovery time"
    bash scriptsbmv2/remote/test_recovery_time.sh > tmp_test_recovery_time.out 2>&1

    echo "[exp10][${exp10_roundnumber}][${exp10_snapshot}] Backup statistics files to ${exp10_output_path}/${exp10_snapshot}"
    sleep 10s
    cp tmp_test_recovery_time.out ${exp10_output_path}/${exp10_snapshot}
    cp ${SWITCH_ROOTPATH}/${exp10_method}/tmp_launchswitchostestbed.out ${exp10_output_path}/${exp10_snapshot}
    cp ${SWITCH_ROOTPATH}/${exp10_method}/tmp_switchos.out ${exp10_output_path}/${exp10_snapshot}
    cp ${CLIENT_ROOTPATH}/${exp10_method}/tmp_server0.out ${exp10_output_path}/${exp10_snapshot}/tmp_server_0.out
    cp ${CLIENT_ROOTPATH}/${exp10_method}/tmp_server1.out ${exp10_output_path}/${exp10_snapshot}/tmp_server_1.out

    python scriptsbmv2/local/calculate_recovery_time_helper.py ${exp10_output_path}/${exp10_snapshot} ${exp10_server_scale}
  done
done
