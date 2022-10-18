if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x
set -e

tmpdirname="farreach"

# Uncomment USE_TOMMYDS_KVS in common/helper.h
echo "update common/helper.h"
cd common
sed -i 's!^//#define USE_TOMMYDS_KVS!#define USE_TOMMYDS_KVS!g' helper.h
cd ..

# Update method/Makefile to link TommyDS
echo "update ${tmpdirname}/Makefile"
cd ${tmpdirname}
rocksdb_server_line=$(sed -n '/^server: server.o $(ROCKSDB_OBJECTS)/=' Makefile)
if [ "x${rocksdb_server_line}" != "x" ]
then
	sed -i ''${rocksdb_server_line}'s!^!#!g'
	sed -i ''$(expr ${rocksdb_server_line} + 1)'s!^!#!g'
	sed -i ''$(expr ${rocksdb_server_line} + 2)'s!^#!!g'
	sed -i ''$(expr ${rocksdb_server_line} + 3)'s!^#!!g'
fi
cd ..

# Replace method/config.ini
echo "update ${tmpdirname}/config.ini (backup original one as ${tmpdirname}/config.ini.bak)"
cd ${tmpdirname}
mv config.ini config.ini.bak
cp configs/config.ini.inmemory config.ini
cd ..

# Sync TommyDS
echo "update scripts/remote/sync.sh"
cd scripts/remote
sed -i 's!##syncfiles_toall tommyds!syncfiles_toall tommyds!g' sync.sh
cd ../../
