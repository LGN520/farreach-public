source ../common.sh

syncfiles_toclient() {
	TMPDIRNAME=$1

	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf projects/NetBuffer/$TMPDIRNAME"

	echo "sync to dl15"
	# NOTE: not --exclude "*.out" for output/*
	rsync -av -e ssh --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer
}

syncfiles_toall(){
	TMPDIRNAME=$1

	ssh ${USER}@bf1 "rm -rf NetBuffer/$TMPDIRNAME"
	ssh ${USER}@bf3 "rm -rf NetBuffer/$TMPDIRNAME"
	ssh ${USER}@dl13 "rm -rf projects/NetBuffer/$TMPDIRNAME"
	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf projects/NetBuffer/$TMPDIRNAME"
	ssh ${USER}@dl16 "rm -rf projects/NetBuffer/$TMPDIRNAME"

	echo "sync to bf1"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@bf1:~/NetBuffer
	echo "sync to bf3"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@bf3:~/NetBuffer
	echo "sync to dl13"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@dl13:~/projects/NetBuffer
	echo "sync to dl15"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer
	echo "sync to dl16"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@dl16:~/projects/NetBuffer
}

cd ../../

# NOTE: comment it only if you have not copied rocksdb to each machine and have not compiled rocksdb in server.
# 	Otherwise, it will overwrite rocksdb in server and you have re-compiled it again.
#syncfiles_toall rocksdb-6.22.1

syncfiles_toall scripts
syncfiles_toclient benchmark
syncfiles_toall common

syncfiles_toall ${DIRNAME}
