source ../scripts/common.sh
#!/use/bin/env bash

DIRNAME="farreach"

echo "kill test_server_rotation.sh"
clientpids=( "$(ps -aux | grep "./test_server_rotation" | grep -v "grep" | awk '{print $2}')" )

if [ ${#clientpids[@]} -gt 0 ]
then
	for i in ${!clientpids[@]}
	do
		if [ "${clientpids[i]}x" != "x" ]
		then
			kill -9 ${clientpids[i]}
		fi
	done
fi

echo "stop servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh server >/dev/null 2>&1"
echo "stop clients"
bash localscripts/localstop.sh client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh client >/dev/null 2>&1"
echo "stop controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localstop.sh controller >/dev/null 2>&1"

echo "kill servers"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null 2>&1"
ssh ${USER}@dl13 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh server >/dev/null 2>&1"
echo "kill clients"
bash localscripts/localkill.sh client >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh client >/dev/null 2>&1"
echo "kill controller"
ssh ${USER}@dl16 "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/localkill.sh controller >/dev/null 2>&1"
