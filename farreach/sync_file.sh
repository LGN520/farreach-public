source ../scripts/common.sh
filename=$1

scp $filename ${USER}@bf1:~/${SWITCH_ROOTPATH}/farreach/$filename
scp $filename ${USER}@dl13:~/${SERVER_ROOTPATH}/farreach/$filename
scp $filename ${USER}@${SECONDARY_CLIENT}:~/${CLIENT_ROOTPATH}/farreach/$filename
scp $filename ${USER}@dl16:~/${SERVER_ROOTPATH}/farreach/$filename
