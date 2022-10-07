source ../common.sh

syncfiles_toclient() {
	DIRNAME=$1

	ssh ssy@dl15 "rm -rf projects/NetBuffer/$DIRNAME"

	echo "sync to dl15"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$DIRNAME ssy@dl15:~/projects/NetBuffer
}

syncfiles_toall(){
	DIRNAME=$1

	ssh ssy@bf1 "rm -rf NetBuffer/$DIRNAME"
	ssh ssy@bf3 "rm -rf NetBuffer/$DIRNAME"
	ssh ssy@dl13 "rm -rf projects/NetBuffer/$DIRNAME"
	ssh ssy@dl15 "rm -rf projects/NetBuffer/$DIRNAME"
	ssh ssy@dl16 "rm -rf projects/NetBuffer/$DIRNAME"

	echo "sync to bf1"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$DIRNAME ssy@bf1:~/NetBuffer
	echo "sync to bf3"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$DIRNAME ssy@bf3:~/NetBuffer
	echo "sync to dl13"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$DIRNAME ssy@dl13:~/projects/NetBuffer
	echo "sync to dl15"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$DIRNAME ssy@dl15:~/projects/NetBuffer
	echo "sync to dl16"
	rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$DIRNAME ssy@dl16:~/projects/NetBuffer
}

cd ../../

# NOTE: comment it only if you have not copied rocksdb to each machine and have not compiled rocksdb in server.
# 	Otherwise, it will overwrite rocksdb in server and you have re-compiled it again.
#syncfiles_toall rocksdb-6.22.1

syncfiles_toclient ycsb
syncfiles_toall common

syncfiles_toall ${DIRNAME}
#syncfiles_toall nocache
#syncfiles_toall netcache
#syncfiles_toall distfarreach
#syncfiles_toall distnocache
#syncfiles_toall distcache
