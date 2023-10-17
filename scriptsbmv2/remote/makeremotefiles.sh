set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

#

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
source scriptsbmv2/local/makeclient.sh # use source to avoid including common.sh twice
# ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scriptsbmv2/local/makeclient.sh"

# NOTE: comment commands of "makeswitchos" for nocache/distnocache
echo "make spine/leaf switchos"
# ssh ${USER}@${LEAFSWITCH} "
cd ${SWITCH_ROOTPATH}; bash scriptsbmv2/local/makeswitchos.sh
#ssh ${USER}@${SPINESWITCH} "cd ${SWITCH_ROOTPATH}; bash scriptsbmv2/local/makeswitchos.sh"

echo "make servers"
# ssh ${USER}@dl16 "
cd ${CLIENT_ROOTPATH}; bash scriptsbmv2/local/makeserver.sh
# ssh ${USER}@dl13 "cd ${CLIENT_ROOTPATH}; bash scriptsbmv2/local/makeserver.sh"
