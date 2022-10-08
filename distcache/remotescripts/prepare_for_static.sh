source ../scripts/common.sh
DIRNAME="distcache"
WORKLOADNAME="synthetic"

# NOTE: NOT required by YCSB clients
echo "sync workload files to another client"
ssh ${USER}@${SECONDARY_CLIENT} "cd projects/NetBuffer/${DIRNAME}; rm -r ${WORKLOADNAME}-run-128"
scp -r ${WORKLOADNAME}-run-128/ ${USER}@${SECONDARY_CLIENT}:~/projects/NetBuffer/${DIRNAME}/ >/dev/null

# NOTE: ONLY required by NetCache/DistCache
echo "sync warmup.out to servers"
scp -r ${WORKLOADNAME}-warmup.out ${USER}@dl16:~/projects/NetBuffer/${DIRNAME}/ >/dev/null
scp -r ${WORKLOADNAME}-warmup.out ${USER}@dl13:~/projects/NetBuffer/${DIRNAME}/ >/dev/null
