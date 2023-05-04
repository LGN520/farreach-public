# Global information unchanged during experiments

USER="ssy"

SWITCH_PRIVATEKEY=".ssh/switch-private-key"
CONNECTION_PRIVATEKEY=".ssh/connection-private-key"

MAIN_CLIENT="dl11" # used in recovery mode
SECONDARY_CLIENT="dl20"

# NOTE: must be consistent with each config.ini
SERVER0="dl21"
SERVER1="dl30"

LEAFSWITCH="bf3"
# NOTE:SPINESWITCH is not used at this stage
# SPINESWITCH="bf3" 

CLIENT_ROOTPATH="/home/${USER}/projects/farreach-private"
SWITCH_ROOTPATH="/home/${USER}/farreach-private"
SERVER_ROOTPATH="/home/${USER}/projects/farreach-private"

# backup rocksdb after loading phase
BACKUPS_ROOTPATH="/tmp/rocksdbbackups"

# experiment
EVALUATION_SCRIPTS_PATH="${CLIENT_ROOTPATH}/scripts"
EVALUATION_OUTPUT_PREFIX="/home/${USER}/results"

# network settings

MAIN_CLIENT_TOSWITCH_IP="10.0.1.11"
MAIN_CLIENT_TOSWITCH_MAC="3c:fd:fe:bb:ca:79"
MAIN_CLIENT_TOSWITCH_FPPORT="11/0"
MAIN_CLIENT_TOSWITCH_PIPEIDX="0"
MAIN_CLIENT_LOCAL_IP="172.16.112.11"

SECONDARY_CLIENT_TOSWITCH_IP="10.0.1.15"
SECONDARY_CLIENT_TOSWITCH_MAC="3c:fd:fe:b5:28:59"
SECONDARY_CLIENT_TOSWITCH_FPPORT="1/0"
SECONDARY_CLIENT_TOSWITCH_PIPEIDX="1"
SECONDARY_CLIENT_LOCAL_IP="172.16.112.15"

SERVER0_TOSWITCH_IP="10.0.1.16"
SERVER0_TOSWITCH_MAC="3c:fd:fe:b5:1f:e1"
SERVER0_TOSWITCH_FPPORT="16/0"
SERVER0_TOSWITCH_PIPEIDX="0"
SERVER0_LOCAL_IP="172.16.112.16"

SERVER1_TOSWITCH_IP="10.0.1.13"
SERVER1_TOSWITCH_MAC="3c:fd:fe:bb:c9:c8"
SERVER1_TOSWITCH_FPPORT="13/0"
SERVER1_TOSWITCH_PIPEIDX="0"
SERVER1_LOCAL_IP="172.16.112.13"

CONTROLLER_LOCAL_IP="172.16.112.16"
SWITCHOS_LOCAL_IP="172.16.112.17"
SWITCH_RECIRPORT_PIPELINE1TO0="2/0"
SWITCH_RECIRPORT_PIPELINE0TO1="12/0"

is_global_included=1
