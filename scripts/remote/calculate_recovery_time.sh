#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x
set -e

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/remote/calculate_recovert_time.sh <roundnumber>"
	exit
fi

roundnumber=$1
methodname=${DIRNAME}
if [ "x${methodname}" != "xfarreach" ]
then
	echo "[ERROR] you can only use this script for FarReach!"
	exit
fi

cd scripts/local/
python calculate_recovery_time_helper.py ${EVALUATION_OUTPUT_PREFIX}/exp9/${roundnumber}/${cache_size} ${server_total_logical_num_for_rotation}
cd ../../
