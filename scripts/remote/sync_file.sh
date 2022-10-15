if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/remote/sync_file.sh <filename/dirname>"
	exit
fi

filename=$1

scp $filename ${USER}@bf1:${SWITCH_ROOTPATH}/$filename
scp $filename ${USER}@bf3:${SWITCH_ROOTPATH}/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/$filename
scp $filename ${USER}@${SERVER0}:${SERVER_ROOTPATH}/$filename
scp $filename ${USER}@${SERVER1}:${SERVER_ROOTPATH}/$filename
