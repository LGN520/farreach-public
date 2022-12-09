# Global information unchanged during experiments

USER="theodorepuyang"

SWITCH_PRIVATEKEY=".ssh/switch-private-key"
CONNECTION_PRIVATEKEY=".ssh/connection-private-key"

MAIN_CLIENT="dl11" # used in recovery mode
SECONDARY_CLIENT="dl15"

# NOTE: must be consistent with each config.ini
SERVER0="dl16"
SERVER1="dl13"

LEAFSWITCH="bf1"
# NOTE:SPINESWITCH is not used at this stage
# SPINESWITCH="bf3" 

CLIENT_ROOTPATH="/home/${USER}/projects/NetBuffer"
SWITCH_ROOTPATH="/home/${USER}/NetBuffer"
SERVER_ROOTPATH="/home/${USER}/projects/NetBuffer"

# backup rocksdb after loading phase
BACKUPS_ROOTPATH="/tmp/rocksdbbackups"

# experiment
EVALUATION_SCRIPTS_PATH="/home/${USER}/projects/NetBuffer/scripts"
EVALUATION_OUTPUT_PREFIX="/home/${USER}/results"

is_global_included=1
