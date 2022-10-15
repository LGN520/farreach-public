source ../scripts/common.sh
DIRNAME="distfarreach"

echo "stop servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null 2>&1"

echo "stop controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh controller >/dev/null 2>&1"

echo "stop reflectors"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/stop_reflector.sh >/dev/null 2>&1"
sudo bash localscripts/stop_reflector.sh

echo "kill servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null 2>&1"

echo "kill controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh controller >/dev/null 2>&1"
