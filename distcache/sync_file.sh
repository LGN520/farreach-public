source ../scripts/common.sh
filename=$1

scp -r $filename ${USER}@bf1:~/NetBuffer/distcache/$filename
scp -r $filename ${USER}@bf3:~/NetBuffer/distcache/$filename
scp -r $filename ${USER}@dl13:~/projects/NetBuffer/distcache/$filename
scp -r $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/distcache/$filename
scp -r $filename ${USER}@dl16:~/projects/NetBuffer/distcache/$filename
