if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

with_controller=0
if [ "x${DIRNAME}" == "xfarreach" ] || [ "x${DIRNAME}" == "xnetcache" ] || [ "x${DIRNAME}" == "xdistfarreach" ] || [ "x${DIRNAME}" == "xdistcache" ]
then
	with_controller=1
fi

source scripts/local/localkill.sh ./test_server_rotation >/dev/null 2>&1

echo "stop servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./server >/dev/null 2>&1"
echo "stop clients"
source bash scripts/local/localstop.sh ./client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localstop.sh ./client >/dev/null 2>&1"
if [ ${with_controller} -eq 1 ]
then
	echo "stop controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh ./controller >/dev/null 2>&1"
fi

if [ "x${DIRNAME}" == "xdistfarreach" ] || [ "x${DIRNAME}" == "xdistcache" ]
then
	echo "stop reflectors"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh /reflector >/dev/null 2>&1"
	sudo bash scripts/local/localstop.sh ./reflector >/dev/null 2>&1
fi

echo "kill servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./server >/dev/null 2>&1"
echo "kill clients"
source scripts/local/localkill.sh ./client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localkill.sh ./client >/dev/null 2>&1"
if [ ${with_controller} -eq 1 ]
then
	echo "kill controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh ./controller >/dev/null 2>&1"
fi

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
