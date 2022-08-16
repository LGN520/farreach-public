#DIRNAME="netreach-v4-lsm"
#DIRNAME="nocache"
#DIRNAME="netcache"
#DIRNAME="distnocache"
#DIRNAME="distfarreach"
DIRNAME="distcache"
##DIRNAME="distfarreachlimit"

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
