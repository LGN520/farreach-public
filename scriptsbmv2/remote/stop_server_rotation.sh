set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

#

source scriptsbmv2/local/localkill.sh ./test_server_rotation >/dev/null 2>&1

echo "stop clients"
source bash scriptsbmv2/local/localstop.sh ycsb >/dev/null 2>&1
# ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scriptsbmv2/local/localstop.sh ycsb >/dev/null 2>&1"

echo "kill clients"
source scriptsbmv2/local/localkill.sh ycsb >/dev/null 2>&1
# ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scriptsbmv2/local/localkill.sh ycsb >/dev/null 2>&1"

# stop and kill server/controller/reflector
source scriptsbmv2/remote/stopservertestbed.sh

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
