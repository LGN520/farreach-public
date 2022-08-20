DIRNAME="netreach-v4-lsm"
WORKLOADNAME="synthetic"

# NOTE: NOT required by YCSB clients
echo "sync workload files to another client"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; rm -r ${WORKLOADNAME}-run-128"
scp -r ${WORKLOADNAME}-run-128/ ssy@dl15:~/projects/NetBuffer/${DIRNAME}/ >/dev/null
