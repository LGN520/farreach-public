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

# Create and sync config.ini for full scale of server rotation
echo "Create and sync config.ini for full scale of server rotation"
source scripts/remote/prepare_server_rotation.sh

# Copy client-side upstream backups from clients to servers 
echo "Launch server and reflector w/ recovery mode"
source scripts/remote/launchservertestbed.sh

# Launch switch
echo "Launch switch data plane"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}/tofino; nohup bash start_switch.sh >tmp_switch.out 2>&1 &"
sleep 10s

# Configure switch and launch switchos
echo "Configure switch data plane and launch switch control plane w/ recovery mode"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}; nohup bash localscripts/launchswitchtestbed.sh recover >tmp_launchswitchtestbed.out 2>&1 &"
sleep 5s

# Close server
echo "Stop server and reflector"
source scripts/remote/stopservertestbed.sh

# Close switchos
echo "Stop switch and switchos"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscripts/stopswitchtestbed.sh >/dev/null 2>&1"

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
