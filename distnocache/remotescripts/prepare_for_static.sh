DIRNAME="distnocache"
WORKLOADNAME="synthetic"

# NOTE: NOT required by YCSB clients
echo "sync workload files to another client"
ssh ${USER}@${SECONDARY_CLIENT} "cd projects/NetBuffer/${DIRNAME}; rm -r ${WORKLOADNAME}-run-128"
scp -r ${WORKLOADNAME}-run-128/ ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/${DIRNAME}/ >/dev/null
