DIRNAME="distcache"

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
cd ../common; make all; cd ../ycsb; bash compile.sh; cd ../$DIRNAME; make all
ssh ${USER}@${SECONDARY_CLIENT} "cd projects/NetBuffer/common; make all; cd ../ycsb; bash compile.sh; cd ../${DIRNAME}; make all"

echo "make spine/leaf switchos"
ssh ssy@bf1 "cd projects/NetBuffer/common; make all; cd ../${DIRNAME}; make clean; make switchos"
ssh ssy@bf3 "cd projects/NetBuffer/common; make all; cd ../${DIRNAME}; make clean; make switchos"

echo "make servers"
ssh ssy@dl16 "cd projects/NetBuffer/common; make all; cd ../${DIRNAME}; make all"
ssh ssy@dl13 "cd projects/NetBuffer/common; make all; cd ../${DIRNAME}; make all"
