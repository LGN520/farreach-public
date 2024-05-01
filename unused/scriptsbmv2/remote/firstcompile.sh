set -x
#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

echo "[COMPILE] first compilation for FarReach"
source scriptsbmv2/remote/setmethod.sh farreach
# source scriptsbmv2/remote/sync.sh
source scriptsbmv2/remote/makeallsoft.sh

echo "[COMPILE] first compilation for NoCache"
source scriptsbmv2/remote/setmethod.sh nocache
# source scriptsbmv2/remote/sync.sh
source scriptsbmv2/remote/makeallsoft.sh

echo "[COMPILE] first compilation for NetCache"
source scriptsbmv2/remote/setmethod.sh netcache
# source scriptsbmv2/remote/sync.sh
source scriptsbmv2/remote/makeallsoft.sh
