function readini() {
	tmpfile=$1
	result=$(awk -F '=' -v tmpsection=[$2] -v tmpkey=$3 '$0==tmpsection {flag = 1; next} /\[/ {flag = 0; next} flag && $1==tmpkey {print $2}' ${tmpfile})
	echo ${result}
}

DIRNAME="farreach"
USER="ssy"

#MAIN_CLIENT="dl11" # NOT used
SECONDARY_CLIENT="dl15"

# NOTE: must be consistent with each config.ini
SERVER0="dl16"
SERVER1="dl13"

#LEAFSWITCH="bf1"
#SPINESWITCH="bf3"

CLIENT_ROOTPATH="/home/${USER}/projects/NetBuffer"
SWITCH_ROOTPATH="/home/${USER}/NetBuffer"
SERVER_ROOTPATH="/home/${USER}/projects/NetBuffer"

# backup rocksdb after loading phase
BACKUPS_ROOTPATH="/tmp/rocksdbbackups"

##### Parse ${DRINAME}/config.ini #####

#CURRENT_ROOTPATH=${CLIENT_ROOTPATH}
#if [ $(hostname) -eq ${SERVER0} ] || [ $(hostname) -eq ${SERVER1} ]
#then
#	CURRENT_ROOTPATH=${SERVER_ROOTPATH}
#elif [ $(hostname) -eq ${LEAFSWITCH} ] || [ $(hostname) -eq ${SPINESWITCH} ]
#	CURRENT_ROOTPATH=${SWITCH_ROOTPATH}
#fi

configfile=${DIRNAME}/config.ini
echo "[COMMON] load configuration from ${configfile}"

# for server rotation (must be consistent with each config.ini)
workloadname=$(readini ${configfile} "global" "workload_name")
workloadmode=$(readini ${configfile} "global" "workload_mode")
dynamicpattern=$(readini ${configfile} "global" "dynamic_ruleprefix")
bottleneck_serveridx=$(readini ${configfile} "global" "bottleneck_serveridx_for_rotation")
server_total_logical_num=$(readini ${configfile} "global" "server_total_logical_num")
server_total_logical_num_for_rotation=$(readini ${configfile} "global" "server_total_logical_num_for_rotation")

is_common_included=1
