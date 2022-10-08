source ../scripts/common.sh
filename=$1

scp $filename ${USER}@bf1:~/NetBuffer/nocache/$filename
scp $filename ${USER}@dl13:~/projects/NetBuffer/nocache/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/nocache/$filename
scp $filename ${USER}@dl16:~/projects/NetBuffer/nocache/$filename
