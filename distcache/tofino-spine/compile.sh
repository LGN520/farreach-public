source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE/pkgsrc/p4-build/
# NOTE: for tofino compiler 9.4.0, main.p4 in P4_PATH must have the same name as P4_NAME
./configure --prefix=$SDE_INSTALL --with-tofino P4_NAME=distcachespine P4_PATH=/home/${USER}/${CLIENT_ROOTPATH}/distcache/tofino-spine/distcachespine.p4  enable_thrift=yes  P4_VERSION=p4-14
make
make install
sed -e 's/TOFINO_SINGLE_DEVICE/distcachespine/g' <$SDE/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in > $SDE_INSTALL/share/p4/targets/tofino/distcachespine.conf
