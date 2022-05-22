DIRNAME="netreach-v4-lsm"

ssh ssy@bf1 "rm -rf NetBuffer/$DIRNAME"
ssh ssy@dl13 "rm -rf projects/NetBuffer/$DIRNAME"

rsync -av -e ssh --exclude "*.out" --exclude "*.bak" ./$DIRNAME ssy@bf1:~/NetBuffer
rsync -av -e ssh --exclude "*.out" --exclude "*.bak" ./$DIRNAME ssy@dl13:~/projects/NetBuffer
