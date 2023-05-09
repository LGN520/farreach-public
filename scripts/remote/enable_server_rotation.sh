#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

echo "[INFO] enable server rotation"
sed -i "s/^\/\/#define SERVER_ROTATION/#define SERVER_ROTATION/g" common/helper.h
source scripts/remote/sync_file.sh common helper.h

echo "[INFO] recompile code for FarReach"
source scripts/remote/setmethod.sh farreach
source scripts/remote/makeallsoft.sh

echo "[INFO] recompile code for NoCache"
source scripts/remote/setmethod.sh nocache
source scripts/remote/makeallsoft.sh

echo "[INFO] recompile code for NetCache"
source scripts/remote/setmethod.sh netcache
source scripts/remote/makeallsoft.sh
