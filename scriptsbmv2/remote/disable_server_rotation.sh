set -x
#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

echo "[COMPILE] enable server rotation"
sed -i "s/^#define SERVER_ROTATION/\/\/#define SERVER_ROTATION/g" common/helper.h
# source scriptsbmv2/remote/sync_file.sh common helper.h
source scriptsbmv2/local/makeclient.sh

cd ${SWITCH_ROOTPATH}/farreach;
make all
cd ${SWITCH_ROOTPATH}/netcache;
make all
cd ${SWITCH_ROOTPATH}/nocache;
make all