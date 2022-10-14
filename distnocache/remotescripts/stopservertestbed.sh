source ../scripts/common.sh
DIRNAME="distnocache"

echo "stop servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null 2>&1"

echo "kill servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null 2>&1"
