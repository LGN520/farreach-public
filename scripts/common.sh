function readini() {     
	tmpfile=$1
	tmpsection=$2
	tmpkey=$3
	result=`awk -F '=' '/\['${tmpsection}'\]/{a=1}a==1&&$1~/'${tmpkey}'/{print $2;exit}' ${tmpfile}`
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
echo "Load configuration from ${configfile}"

# for server rotation (must be consistent with each config.ini)
workloadmode=$(readini ${configfile} "global" "workload_mode")
bottleneck_serveridx=$(readini ${configfile} "global" "bottleneck_serveridx_for_rotation")
server_total_logical_num=$(readini ${configfile} "global" "server_total_logical_num")
server_total_logical_num_for_rotation=$(readini ${configfile} "global" "server_total_logical_num_for_rotation")
