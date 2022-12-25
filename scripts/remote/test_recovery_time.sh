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

# Collect recovery information for servers
#echo "Collect recovery information for servers"
#begin_time_2=`date +%s.%N`
#ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/fetchbackup_client2server.sh"
#end_time_2_1=`date +%s.%N`
#ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/fetchbackup_client2server.sh"
#end_time_2_2=`date +%s.%N`
#collect_time_2_1=$(getTiming ${begin_time_2} ${end_time_2_1})
#collect_time_2_2=$(getTiming ${end_time_2_1} ${end_time_2_2})
#avg_collect_time_2=$(echo "(${collect_time_2_1} + ${collect_time_2_2}) / 2.0" | bc)
#echo "[Statistics] collect time server: ${avg_collect_time_2} s"

# Collect recovery information for servers
echo "Collect recovery information for servers"
begin_time_2=`date +%s.%N`
# Let server0 and server1 collect client-side backup files simultaneously
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/fetchbackup_client2server.sh"; ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; bash localscripts/fetchbackup_client2server.sh"
end_time_2=`date +%s.%N`
collect_time_2=$(getTiming ${begin_time_2} ${end_time_2})
echo "[Statistics] collect time server: ${collect_time_2} s"

# Create and sync config.ini for full scale of server rotation
echo "Create and sync config.ini for full scale of server rotation"
if [ "x${workloadmode}" == "x0" ]
then
	source scripts/remote/prepare_server_rotation.sh
else
	# TODO: create and sync correct config.ini for the dynamic pattern
	echo "[ERROR] not support dynamic pattern now"
	exit
fi

# Launch servers w/ recovery mode
echo "Launch servers w/ recovery mode and reflector"
source scripts/remote/launchservertestbed.sh recover

# Collect recovery information for in-switch cache
echo "Collect recovery information for in-switch cache"
begin_time_1=`date +%s.%N`
# Fetch all to switch
ssh ${USER}@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscripts/fetchall_all2switch.sh"
# Fetch client-side backups to server
end_time_1=`date +%s.%N`
collect_time_1=$(getTiming ${begin_time_1} ${end_time_1})
echo "[Statistics] collect time switchos: ${collect_time_1} s"

# Launch switch
echo "Launch switch data plane"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${DIRNAME}/tofino; rm tmp_switch.out; nohup bash start_switch.sh >tmp_switch.out 2>&1 &"
sleep 10s

# Configure switch and launch switchos w/ recovery mode
echo "Configure switch data plane and launch switch control plane w/ recovery mode"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${DIRNAME}; rm tmp_launchswitchostestbed.out; bash localscripts/launchswitchostestbed.sh recover >tmp_launchswitchostestbed.out 2>&1"
sleep 5s

# Close server
echo "Stop server and reflector"
source scripts/remote/stopservertestbed.sh

# Close switchos
echo "Stop switch and switchos"
ssh -i /home/${USER}/${SWITCH_PRIVATEKEY} root@${LEAFSWITCH} "cd ${SWITCH_ROOTPATH}/${DIRNAME}; bash localscripts/stopswitchtestbed.sh >/dev/null 2>&1"

echo "Resume ${DIRNAME}/config.ini with ${DIRNAME}/config.ini.bak if any"
mv ${DIRNAME}/config.ini.bak ${DIRNAME}/config.ini
