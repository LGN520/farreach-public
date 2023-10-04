if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

echo "For ${DIRNAME}:"

# Stop and kill clients
echo "Stop and kill clients..."
source bash scripts/local/localstop.sh ycsb >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localstop.sh ycsb >/dev/null 2>&1"
source scripts/local/localkill.sh ycsb >/dev/null 2>&1
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}; bash scripts/local/localkill.sh ycsb >/dev/null 2>&1"

# Stop and kill servers and controller (w/ reflector)
echo "Stop and kill servers..."
bash scripts/remote/stopservertestbed.sh

# Stop and kill switch
echo "Stop and kill switch data plane and switch OS (as well as daemon processes)..."
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscripts/stopswitchtestbed.sh"

# Stop and kill processes in main client
echo "Stop run_exps if any..."
source scripts/local/localkill.sh run_exp >/dev/null 2>&1
echo "Stop warmup_client is any"
source scripts/local/localkill.sh warmup_client >/dev/null 2>&1
echo "Stop test_server_rotation if any..."
bash scripts/remote/stop_server_rotation.sh
echo "Stop test_dynamic if any..."
souce scripts/local/localkill.sh test_dynamic >/dev/null 2>&1
echo "Stop "
