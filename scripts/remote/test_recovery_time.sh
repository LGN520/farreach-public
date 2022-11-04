if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

if [ "x${DIRNAME}" != "xfarreach" ]
then
	echo "[ERROR] you should change DIRNAME as farreach in scripts/common.sh before running test_recovery_time.sh"
	exit
fi

# Launch switch
echo "Launch switch data plane"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}/tofino; nohup bash start_switch.sh >tmp_switch.out 2>&1 &"
sleep 15s

# Copy in-switch snapshot id/data from controller to switch
echo "Copy in-switch snapshot from controller to switch"
ssh ${USER}@bf1 "mkdir -p /tmp/${DIRNAME}"
scp ${USER}@${SERVER0}:/tmp/${DIRNAME}/controller.snapshot* ${USER}@bf1:/tmp/${DIRNAME}

# Configure switch
echo "Configure switch data plane and launch switch control plane w/ recovery mode"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}; nohup bash localscripts/launchswitchtestbed.sh recover >tmp_launchswitchtestbed.out 2>&1 &"
sleep 15s
