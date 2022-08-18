DIRNAME="distnocache"

set -e

# NOTE: you need to use ../sync.sh to sync source code first

echo "make clients"
make
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; make"

echo "make servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; make"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; make"
