#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

echo "[INFO] make software code in main client"
source scripts/local/makeclient.sh

echo "[INFO] make software code in secondary client"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/makeclient.sh"

echo "[INFO] make software code in first server"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/makeserver.sh"

echo "[INFO] make software code in second server"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/makeserver.sh"

echo "[INFO] make software code in switch"
ssh ${USER}@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}; bash scripts/local/makeswitchos.sh"
