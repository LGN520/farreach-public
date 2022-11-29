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

function getTiming(){
    start=$1
    end=$2
 
    start_s=`echo $start | cut -d '.' -f 1`
    start_ns=`echo $start | cut -d '.' -f 2`
    end_s=`echo $end | cut -d '.' -f 1`
    end_ns=`echo $end | cut -d '.' -f 2`
 
    time_micro=$(( (10#$end_s-10#$start_s)*1000000 + (10#$end_ns/1000 - 10#$start_ns/1000) ))
    #time_ms=`expr $time_micro/1000  | bc `
 
    #echo "$time_micro microseconds"
    #echo "$time_ms ms"
	echo $(echo "scale=4; ${time_micro} / 1000.0 / 1000.0" | bc)
}

# Collect recovery information
echo "Collect recovery information among clients/servers/switchos"
begin_time_1=`date +%s.%N`
# Fetch all to switch
ssh ${USER}@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscripts/fetchall_all2switch.sh >/dev/null 2>&1"
# Fetch client-side backups to server
end_time_1=`date +%s.%N`
begin_time_2=`date +%s.%N`
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/fetchbackup_client2server.sh >/dev/null 2>&1"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/fetchbackup_client2server.sh >/dev/null 2>&1"
end_time_2=`date +%s.%N`
collect_time_1=$(getTiming ${begin_time_1} ${end_time_1})
collect_time_2=$(getTiming ${begin_time_2} ${end_time_2})
echo "[Statistics] collect time switchos: ${collect_time_1} s"
echo "[Statistics] collect time server: ${collect_time_2} s"

# Create and sync config.ini for full scale of server rotation
echo "Create and sync config.ini for full scale of server rotation"
source scripts/remote/prepare_server_rotation.sh

# Copy client-side upstream backups from clients to servers 
echo "Launch server and reflector w/ recovery mode"
source scripts/remote/launchservertestbed.sh recover

# Launch switch
echo "Launch switch data plane"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}/tofino; rm tmp_switch.out; nohup bash start_switch.sh >tmp_switch.out 2>&1 &"
sleep 10s

# Configure switch and launch switchos
echo "Configure switch data plane and launch switch control plane w/ recovery mode"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}; rm tmp_launchswitchostestbed.out; bash localscripts/launchswitchostestbed.sh recover >tmp_launchswitchostestbed.out 2>&1"
sleep 5s

# Close server
echo "Stop server and reflector"
source scripts/remote/stopservertestbed.sh

# Close switchos
echo "Stop switch and switchos"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@bf1 "cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscripts/stopswitchtestbed.sh >/dev/null 2>&1"

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
