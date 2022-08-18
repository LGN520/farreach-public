DIRNAME="netreach-v4-lsm"

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
make
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; make"

echo "make spine/leaf switchos"
ssh ssy@bf1 "cd NetBuffer/${DIRNAME}; make clean; make switchos"

echo "make servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; make"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; make"
