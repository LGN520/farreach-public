DIRNAME="distnocache"
WORKLOADNAME="synthetic"

# NOTE: NOT required by YCSB clients
echo "sync workload files to another client"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; rm -r ${WORKLOADNAME}-run-128"
scp -r ${WORKLOADNAME}-run-128/ ssy@dl15:~/projects/NetBuffer/${DIRNAME}/ >/dev/null

echo "sync dynamic rulemaps to another client"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; rm -r dynamicrules"
scp -r dynamicrules/ ssy@dl15:~/projects/NetBuffer/${DIRNAME}/ >/dev/null
