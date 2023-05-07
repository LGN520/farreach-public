if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

function syncfiles_toclient() {
	TMPDIRNAME=$1
	TMPFILENAME=$2

	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $CLIENT_ROOTPATH/$TMPDIRNAME"

	echo "sync $TMPDIRNAME/$TMPFILENAME to ${SECONDARY_CLIENT}"
	rsync -av -e ssh --exclude "*.out" --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/$TMPDIRNAME >/dev/null
}

function syncfiles_toall(){
	TMPDIRNAME=$1
	TMPFILENAME=$2

	ssh ${USER}@${LEAFSWITCH} "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
	#ssh ${USER}@${SPINESWITCH} "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $CLIENT_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SERVER0} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SERVER1} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"

	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${LEAFSWITCH}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${LEAFSWITCH}:${SWITCH_ROOTPATH}/${TMPDIRNAME} >/dev/null
	#echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SPINESWITCH}"
	#rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SPINESWITCH}:${SWITCH_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SECONDARY_CLIENT}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SERVER0}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER0}:${SERVER_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SERVER1}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER1}:${SERVER_ROOTPATH}/$TMPDIRNAME >/dev/null
}

function syncoutputfiles_toall(){
	TMPDIRNAME=$1
	TMPFILENAME=$2

	ssh ${USER}@${LEAFSWITCH} "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
	#ssh ${USER}@${SPINESWITCH} "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $CLIENT_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SERVER0} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SERVER1} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"

	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${LEAFSWITCH}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${LEAFSWITCH}:${SWITCH_ROOTPATH}/${TMPDIRNAME} >/dev/null
	#echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SPINESWITCH}"
	#rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SPINESWITCH}:${SWITCH_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SECONDARY_CLIENT}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SERVER0}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER0}:${SERVER_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SERVER1}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER1}:${SERVER_ROOTPATH}/$TMPDIRNAME >/dev/null
}

syncfiles_toall scripts \*
syncfiles_toclient benchmark/inswitchcache-java-lib/ \*
syncfiles_toclient benchmark/ycsb/ \*
syncfiles_toall common \*

syncfiles_toall ${DIRNAME} \*

# sync hotkey files to servers for netcache/distcache to resume cached keyset under server rotation
syncoutputfiles_toall benchmark/output \*-hotest.out
