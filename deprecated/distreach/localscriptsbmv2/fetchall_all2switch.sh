
if [ "x${is_common_included}" != "x1" ]
then
	cd ..
	source scriptsbmv2/common.sh
	if [ "x${DIRNAME}" != "xfarreach" ]
	then
		echo "[ERROR] you should change DIRNAME as farreach in scripts/common.sh before running fetchsnapshotandmaxseq_controllerandserver2switch.sh"
		exit
	fi
	cd ${DIRNAME}
fi

# 

if [ "x${DIRNAME}" != "xfarreach" ]
then
	echo "[ERROR] DIRNAME should be farreach in scripts/common.sh for fetchall_all2switch.sh"
	exit
fi

mkdir -p /tmp/${DIRNAME}
rm -r /tmp/${DIRNAME}/*

