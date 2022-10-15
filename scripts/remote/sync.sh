if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

function syncfiles_toclient() {
	TMPDIRNAME=$1

	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME"

	echo "sync to ${SECONDARY_CLIENT}"
	rsync -av -e ssh --exclude "*.out" --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH} >/dev/null
}

function syncfiles_toall(){
	TMPDIRNAME=$1

	ssh ${USER}@bf1 "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME"
	ssh ${USER}@bf3 "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME"
	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME"
	ssh ${USER}@${SERVER0} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME"
	ssh ${USER}@${SERVER1} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME"

	echo "sync to bf1"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@bf1:${SWITCH_ROOTPATH} >/dev/null
	echo "sync to bf3"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@bf3:${SWITCH_ROOTPATH} >/dev/null
	echo "sync to ${SECONDARY_CLIENT}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH} >/dev/null
	echo "sync to ${SERVER0}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@${SERVER0}:${SERVER_ROOTPATH} >/dev/null
	echo "sync to ${SERVER1}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@${SERVER1}:${SERVER_ROOTPATH} >/dev/null
}

# NOTE: comment it only if you have not copied rocksdb to each machine and have not compiled rocksdb in server.
# 	Otherwise, it will overwrite rocksdb in server and you have re-compiled it again.
#syncfiles_toall rocksdb-6.22.1

syncfiles_toall scripts
syncfiles_toclient benchmark
syncfiles_toall common

syncfiles_toall ${DIRNAME}
