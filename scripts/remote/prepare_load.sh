if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

tmpsrcdirname="recordload"
tmpdstdirname="nocache"
tmpfilename="config.ini"

ssh ${USER}@bf1 "rm -r ${SWITCH_ROOTPATH}/${tmpdstdirname}/${tmpfilename}"
ssh ${USER}@bf3 "rm -r ${SWITCH_ROOTPATH}/${tmpdstdirname}/${tmpfilename}"
ssh ${USER}@${SECONDARY_CLIENT} "rm -r ${CLIENT_ROOTPATH}/${tmpdstdirname}/${tmpfilename}"
ssh ${USER}@${SERVER0} "rm -r ${SERVER_ROOTPATH}/${tmpdstdirname}/${tmpfilename}"
ssh ${USER}@${SERVER1} "rm -r ${SERVER_ROOTPATH}/${tmpdstdirname}/${tmpfilename}"

echo "Copy ${tmpsrcdirname}/${tmpfilename} to ${tmpdstdirname}/${tmpfilename} in bf1"
scp ${tmpsrcdirname}/${tmpfilename} ${USER}@bf1:${SWITCH_ROOTPATH}/${tmpdstdirname}
echo "Copy ${tmpsrcdirname}/${tmpfilename} to ${tmpdstdirname}/${tmpfilename} in bf3"
scp ${tmpsrcdirname}/${tmpfilename} ${USER}@bf3:${SWITCH_ROOTPATH}/${tmpdstdirname}
echo "Copy ${tmpsrcdirname}/${tmpfilename} to ${tmpdstdirname}/${tmpfilename} in ${SECONDARY_CLIENT}"
scp ${tmpsrcdirname}/${tmpfilename} ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${tmpdstdirname}
echo "Copy ${tmpsrcdirname}/${tmpfilename} to ${tmpdstdirname}/${tmpfilename} in ${SERVER0}"
scp ${tmpsrcdirname}/${tmpfilename} ${USER}@${SERVER0}:${SERVER_ROOTPATH}/${tmpdstdirname}
echo "Copy ${tmpsrcdirname}/${tmpfilename} to ${tmpdstdirname}/${tmpfilename} in ${SERVER1}"
scp ${tmpsrcdirname}/${tmpfilename} ${USER}@${SERVER1}:${SERVER_ROOTPATH}/${tmpdstdirname}
