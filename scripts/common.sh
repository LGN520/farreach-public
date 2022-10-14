DIRNAME="farreach"
USER="ssy"

#MAIN_CLIENT="dl11" # NOT used
SECONDARY_CLIENT="dl15"

CLIENT_ROOTPATH="/home/${USER}/projects/NetBuffer"
SWITCH_ROOTPATH="/home/${USER}/NetBuffer"
SERVER_ROOTPATH="/home/${USER}/projects/NetBuffer"

# backup rocksdb after loading phase
BACKUPS_ROOTPATH="/home/rocksdbbackups"

# for server rotation (must be consistent with each config.ini)
bottleneck_serveridx=123
server_total_logical_num_for_rotation=128
