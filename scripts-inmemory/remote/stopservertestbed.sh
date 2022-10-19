if [ "x${is_common_included}" != "x1" ]
then
	source scripts-inmemory/common.sh
fi

#set -x

echo "stop servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localstop.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localstop.sh ./server >/dev/null 2>&1"

if [ ${with_controller} -eq 1 ]
then
	echo "stop controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localstop.sh ./controller >/dev/null 2>&1"
fi

if [ ${with_reflector} -eq 1 ]
then
	echo "stop reflectors"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localstop.sh ./reflector >/dev/null 2>&1"
	sudo source scripts-inmemory/local/localstop.sh ./reflector >/dev/null 2>&1
fi

#sleep 1s

echo "kill servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localkill.sh ./server >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localkill.sh ./server >/dev/null 2>&1"

if [ ${with_controller} -eq 1 ]
then
	echo "kill controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localkill.sh ./controller >/dev/null 2>&1"
fi

if [ ${with_reflector} -eq 1 ]
then
	echo "kill reflector"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts-inmemory/local/localkill.sh ./reflector >/dev/null 2>&1"
	sudo source scripts-inmemory/local/localkill.sh ./reflector >/dev/null 2>&1
fi
