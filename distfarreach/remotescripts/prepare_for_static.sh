DIRNAME="distfarreach"
WORKLOADNAME="synthetic"

# NOTE: NOT required by YCSB clients
echo "sync workload files to another client"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; rm -r ${WORKLOADNAME}-run-128"
scp -r ${WORKLOADNAME}-run-128/ ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${DIRNAME}/ >/dev/null
