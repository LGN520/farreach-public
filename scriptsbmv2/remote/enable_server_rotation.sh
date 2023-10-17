set -x
#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

echo "[COMPILE] enable server rotation"
sed -i "s/^\/\/#define SERVER_ROTATION/#define SERVER_ROTATION/g" common/helper.h
source scriptsbmv2/remote/sync_file.sh common helper.h

echo "[COMPILE] recompile code for FarReach"
source scriptsbmv2/remote/setmethod.sh farreach
source scriptsbmv2/remote/makeallsoft.sh

echo "[COMPILE] recompile code for NoCache"
source scriptsbmv2/remote/setmethod.sh nocache
source scriptsbmv2/remote/makeallsoft.sh

echo "[COMPILE] recompile code for NetCache"
source scriptsbmv2/remote/setmethod.sh netcache
source scriptsbmv2/remote/makeallsoft.sh
