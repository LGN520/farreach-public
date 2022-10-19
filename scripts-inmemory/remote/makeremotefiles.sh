if [ "x${is_common_included}" != "x1" ]
then
	source scripts-inmemory/common.sh
fi

#set -x

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
source scripts/local/makeclient.sh # use source to avoid including common.sh twice
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/makeclient.sh"

# NOTE: comment commands of "makeswitchos" for nocache/distnocache
echo "make spine/leaf switchos"
ssh ${USER}@bf1 "cd ${SWITCH_ROOTPATH}; bash scripts/local/makeswitchos.sh"
ssh ${USER}@bf3 "cd ${SWITCH_ROOTPATH}; bash scripts/local/makeswitchos.sh"

echo "make servers"
ssh ${USER}@dl16 "cd ${CLIENT_ROOTPATH}; bash scripts/local/makeserver.sh"
ssh ${USER}@dl13 "cd ${CLIENT_ROOTPATH}; bash scripts/local/makeserver.sh"
