source ../scripts/common.sh
filename=$1

scp -r $filename ${USER}@bf1:~${SWITCH_ROOTPATH}distcache/$filename
scp -r $filename ${USER}@bf3:~${SWITCH_ROOTPATH}distcache/$filename
scp -r $filename ${USER}@dl13:~/${SERVER_ROOTPATH}/distcache/$filename
scp -r $filename ${USER}@${SECONDARY_CLIENT}:~/${CLIENT_ROOTPATH}/distcache/$filename
scp -r $filename ${USER}@dl16:~/${SERVER_ROOTPATH}/distcache/$filename
