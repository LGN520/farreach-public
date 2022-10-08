source ../scripts/common.sh
filename=$1

scp $filename ${USER}@bf1:~/NetBuffer/netcache/$filename
scp $filename ${USER}@dl13:~/projects/NetBuffer/netcache/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/netcache/$filename
scp $filename ${USER}@dl16:~/projects/NetBuffer/netcache/$filename
