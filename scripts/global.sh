# Global information unchanged during experiments

USER="theodorepuyang"

SWITCH_PRIVATEKEY=".ssh/id_rsa_forbf1"

MAIN_CLIENT="dl11" # used in recovery mode
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

is_global_included=1
