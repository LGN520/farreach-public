source ../common.sh

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
cd ../local; bash makeclient.sh
ssh ${USER}@dl15 "cd ${CLIENT_ROOTPATH}/scripts/local; bash makeclient.sh"

echo "make spine/leaf switchos"
ssh ${USER}@bf1 "cd ${SWITCH_ROOTPATH}/scripts/local; bash makeswitchos.sh"
ssh ${USER}@bf3 "cd ${SWITCH_ROOTPATH}/scripts/local; bash makeswitchos.sh"

echo "make servers"
ssh ${USER}@dl16 "cd ${CLIENT_ROOTPATH}/scripts/local; bash makeserver.sh"
ssh ${USER}@dl13 "cd ${CLIENT_ROOTPATH}/scripts/local; bash makeserver.sh"
