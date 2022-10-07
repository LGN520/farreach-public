source ../common.sh

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
cd ../local; bash makeclient.sh
ssh ssy@dl15 "cd ${CLIENT_ROOTPATH}/scripts/local; bash makeclient.sh"

echo "make spine/leaf switchos"
ssh ssy@bf1 "cd ${SWITCH_ROOTPATH}/scripts/local; bash makeswitchos.sh"
ssh ssy@bf3 "cd ${SWITCH_ROOTPATH}/scripts/local; bash makeswitchos.sh"

echo "make servers"
ssh ssy@dl16 "cd ${CLIENT_ROOTPATH}/scripts/local; bash makeserver.sh"
ssh ssy@dl13 "cd ${CLIENT_ROOTPATH}/scripts/local; bash makeserver.sh"
