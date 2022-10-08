source ../scripts/common.sh
DIRNAME="netcache"
WORKLOADNAME="synthetic"

# NOTE: NOT required by YCSB clients
echo "sync workload files to another client"
ssh ${USER}@${SECONDARY_CLIENT} "cd ${CLIENT_ROOTPATH}/${DIRNAME}; rm -r ${WORKLOADNAME}-run-128"
scp -r ${WORKLOADNAME}-run-128/ ${USER}@${SECONDARY_CLIENT}:~/${CLIENT_ROOTPATH}/${DIRNAME}/ >/dev/null

# NOTE: ONLY required by NetCache/DistCache
echo "sync warmup.out to servers"
scp -r ${WORKLOADNAME}-warmup.out ${USER}@dl16:~/${SERVER_ROOTPATH}/${DIRNAME}/ >/dev/null
scp -r ${WORKLOADNAME}-warmup.out ${USER}@dl13:~/${SERVER_ROOTPATH}/${DIRNAME}/ >/dev/null
