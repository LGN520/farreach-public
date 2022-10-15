if [ ${is_common_included} -ne 1 ]
then
	source scripts/common.sh
fi

set -x

echo "stop servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh server >/dev/null"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh server >/dev/null"

if [ ${DRINAME} == "farreach" ] || [ ${DIRNAME} == "netcache" ] || [ ${DIRNAME} == "distcache" ] || [ ${DIRNAME} == "distfarreach"]
then
	echo "stop controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh controller >/dev/null"
fi

if [ ${DIRNAME} == "distcache" ] || [ ${DIRNAME} == "distfarreach"]
then
	echo "stop reflectors"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localstop.sh reflector >/dev/null"
	sudo bash scripts/local/localstop.sh reflector
fi

echo "kill servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh server >/dev/null"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh server >/dev/null"

if [ ${DRINAME} == "farreach" ] | [ ${DIRNAME} == "netcache" ] || [ ${DIRNAME} == "distcache" ] || [ ${DIRNAME} == "distfarreach"]
then
	echo "kill controller"
	ssh ${USER}${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh controller >/dev/null"
fi

if [ ${DIRNAME} == "distcache" ] || [ ${DIRNAME} == "distfarreach"]
then
	echo "kill reflector"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}; bash scripts/local/localkill.sh reflector >/dev/null"
	sudo bash scripts/local/localkill.sh reflector
fi
