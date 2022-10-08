DIRNAME="nocache"

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
cd ../common; make all; cd ../ycsb; bash compile.sh; cd ../$DIRNAME; make all
ssh ${USER}@${SECONDARY_CLIENT} "cd projects/NetBuffer/common; make all; cd ../ycsb; bash compile.sh; cd ../${DIRNAME}; make all"

echo "make servers"
ssh ssy@dl16 "cd projects/NetBuffer/common; make all; cd ../${DIRNAME}; make all"
ssh ssy@dl13 "cd projects/NetBuffer/common; make all; cd ../${DIRNAME}; make all"
