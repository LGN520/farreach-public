DIRNAME="distcache"
WORKLOADNAME="synthetic"

# NOTE: NOT required by YCSB clients
echo "sync workload files to another client"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; rm -r ${WORKLOADNAME}-run-128"
scp -r ${WORKLOADNAME}-run-128/ ssy@dl15:~/projects/NetBuffer/${DIRNAME}/ >/dev/null

# NOTE: ONLY required by NetCache/DistCache
echo "sync warmup.out to servers"
scp -r ${WORKLOADNAME}-warmup.out ssy@dl16:~/projects/NetBuffer/${DIRNAME}/ >/dev/null
scp -r ${WORKLOADNAME}-warmup.out ssy@dl13:~/projects/NetBuffer/${DIRNAME}/ >/dev/null
