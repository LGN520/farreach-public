if [ "x${is_common_included}" != "x1" ]
then
	source scripts-inmemory/common.sh
fi

#set -x

source scripts-inmemory/local/localkill.sh ./test_server_rotation >/dev/null 2>&1

echo "stop clients"
source bash scripts-inmemory/local/localstop.sh ycsb >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts-inmemory/local/localstop.sh ycsb >/dev/null 2>&1"

echo "kill clients"
source scripts-inmemory/local/localkill.sh ycsb >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts-inmemory/local/localkill.sh ycsb >/dev/null 2>&1"

# stop and kill server/controller/reflector
source scripts-inmemory/remote/stopservertestbed.sh

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
