#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

echo "[COMPILE] first compilation for FarReach"
source scripts/remote/setmethod.sh farreach
source scripts/remote/sync.sh
source scripts/remote/makeallsoft.sh

echo "[COMPILE] first compilation for NoCache"
source scripts/remote/setmethod.sh nocache
source scripts/remote/sync.sh
source scripts/remote/makeallsoft.sh

echo "[COMPILE] first compilation for NetCache"
source scripts/remote/setmethod.sh netcache
source scripts/remote/sync.sh
source scripts/remote/makeallsoft.sh
