DIRNAME="netreach-v4-lsm"
#DIRNAME="nocache"
#DIRNAME="netcache"

ssh ssy@bf1 "rm -rf NetBuffer/$DIRNAME"
ssh ssy@dl13 "rm -rf projects/NetBuffer/$DIRNAME"
ssh ssy@dl15 "rm -rf projects/NetBuffer/$DIRNAME"
ssh ssy@dl16 "rm -rf projects/NetBuffer/$DIRNAME"

rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" ./$DIRNAME ssy@bf1:~/NetBuffer
rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" ./$DIRNAME ssy@dl13:~/projects/NetBuffer
rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" ./$DIRNAME ssy@dl15:~/projects/NetBuffer
rsync -av -e ssh --exclude "*.out*" --exclude "*.bak" --exclude "*.o" --exclude "*.d" ./$DIRNAME ssy@dl16:~/projects/NetBuffer
