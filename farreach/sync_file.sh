source ../scripts/common.sh
filename=$1

scp $filename ${USER}@bf1:~/NetBuffer/farreach/$filename
scp $filename ${USER}@dl13:~/projects/NetBuffer/farreach/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/farreach/$filename
scp $filename ${USER}@dl16:~/projects/NetBuffer/farreach/$filename
