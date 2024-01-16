set -x
#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scriptsdist/common.sh
fi

echo "[COMPILE] first compilation for FarReach"
source scriptsdist/remote/setmethod.sh farreach
# source scriptsdist/remote/sync.sh
source scriptsdist/remote/makeallsoft.sh

echo "[COMPILE] first compilation for NoCache"
source scriptsdist/remote/setmethod.sh nocache
# source scriptsdist/remote/sync.sh
source scriptsdist/remote/makeallsoft.sh

echo "[COMPILE] first compilation for NetCache"
source scriptsdist/remote/setmethod.sh netcache
# source scriptsdist/remote/sync.sh
source scriptsdist/remote/makeallsoft.sh
