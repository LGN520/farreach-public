#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/remote/set_method.sh <methodname>"
	exit
fi

methodname=$1
if [ "x${methodname}" != "xfarreach" ] && [ "x${methodname}" != "xnocache" ] && [ "x${methodname}" != "xnetcache" ]
then
	echo "[ERROR] <methodname> must be farreach, nocache, or netcache"
	exit
fi

sed -i "s/^DIRNAME=.*/DIRNAME=\"${methodname}\"/g" scripts/common.sh
source scripts/remote/sync_file.sh scripts common.sh
