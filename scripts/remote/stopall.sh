if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

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
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscripts/stopswitchtestbed.sh"
