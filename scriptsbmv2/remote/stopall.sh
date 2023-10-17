set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

echo "For ${DIRNAME}:"

# Stop and kill clients
echo "Stop and kill clients..."
bash scriptsbmv2/local/localstop.sh ycsb >/dev/null 2>&1
bash scriptsbmv2/local/localkill.sh ycsb >/dev/null 2>&1"

# Stop and kill servers and controller (w/ reflector)
echo "Stop and kill servers..."
bash scriptsbmv2/remote/stopservertestbed.sh

# Stop and kill switch
echo "Stop and kill switch data plane and switch OS (as well as daemon processes)..."
# ?
# ssh -i /root/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "
cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscriptsbmv2/stopswitchtestbed.sh

# Stop and kill processes in main client
echo "Stop run_exps if any..."
source scriptsbmv2/local/localkill.sh run_exp >/dev/null 2>&1
echo "Stop warmup_client is any"
source scriptsbmv2/local/localkill.sh warmup_client >/dev/null 2>&1
echo "Stop test_server_rotation if any..."
bash scriptsbmv2/remote/stop_server_rotation.sh
echo "Stop test_dynamic if any..."
souce scriptsbmv2/local/localkill.sh test_dynamic >/dev/null 2>&1
echo "Stop "
