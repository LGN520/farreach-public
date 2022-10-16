if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

if [ $# -ne 2 ]
then
	echo "Usage: bash scripts/remote/sync_file.sh <dirname> <filename>"
	exit
fi

TMPDIRNAME=$1
TMPFILENAME=$2

ssh ${USER}@bf1 "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
ssh ${USER}@bf3 "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $CLIENT_ROOTPATH/$TMPDIRNAME"
ssh ${USER}@${SERVER0} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"
ssh ${USER}@${SERVER1} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"

echo "Copy $TMPDIRNAME/$TMPFILENAME to bf1"
scp $TMPDIRNAME/$TMPFILENAME ${USER}@bf1:${SWITCH_ROOTPATH}/$TMPDIRNAME
echo "Copy $TMPDIRNAME/$TMPFILENAME to bf3"
scp $TMPDIRNAME/$TMPFILENAME ${USER}@bf3:${SWITCH_ROOTPATH}/$TMPDIRNAME
echo "Copy $TMPDIRNAME/$TMPFILENAME to ${SECONDARY_CLIENT}"
scp $TMPDIRNAME/$TMPFILENAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/$TMPDIRNAME
echo "Copy $TMPDIRNAME/$TMPFILENAME to ${SERVER0}"
scp $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER0}:${SERVER_ROOTPATH}/$TMPDIRNAME
echo "Copy $TMPDIRNAME/$TMPFILENAME to ${SERVER1}"
scp $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER1}:${SERVER_ROOTPATH}/$TMPDIRNAME
