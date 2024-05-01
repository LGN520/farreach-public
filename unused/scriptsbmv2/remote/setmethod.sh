set -x
#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

if [ $# -ne 1 ]
then
	echo "Usage: bash scriptsbmv2/remote/set_method.sh <methodname>"
	exit
fi

methodname=$1
if [ "x${methodname}" != "xfarreach" ] && [ "x${methodname}" != "xnocache" ] && [ "x${methodname}" != "xnetcache" ]
then
	echo "[ERROR] <methodname> must be farreach, nocache, or netcache"
	exit
fi

sed -i "s/^DIRNAME=.*/DIRNAME=\"${methodname}\"/g" scriptsbmv2/common.sh

