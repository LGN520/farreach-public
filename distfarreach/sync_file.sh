source ../scripts/common.sh
filename=$1

scp $filename ${USER}@bf1:~/NetBuffer/distfarreach/$filename
scp $filename ${USER}@bf3:~/NetBuffer/distfarreach/$filename
scp $filename ${USER}@dl13:~/projects/NetBuffer/distfarreach/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/distfarreach/$filename
scp $filename ${USER}@dl16:~/projects/NetBuffer/distfarreach/$filename
