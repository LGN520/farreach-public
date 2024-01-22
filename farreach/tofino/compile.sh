#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd farreach/tofino

# cd $SDE/pkgsrc/p4-build/
# # NOTE: for tofino compiler 9.4.0, main.p4 in P4_PATH must have the same name as P4_NAME

# ./configure --prefix=$SDE_INSTALL --with-tofino --with-pd P4_NAME=farreach P4_PATH=${SWITCH_ROOTPATH}/farreach/tofino/farreach.p4  enable_thrift=yes  P4_VERSION=p4-16
# make
# make install
# # # sed -e 's/TOFINO_SINGLE_DEVICE/farreach/g' <$SDE/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in > $SDE_INSTALL/share/p4/targets/tofino/farreach.conf
# # # 
$SDE/p4_build.sh ${SWITCH_ROOTPATH}/farreach/tofino/netbufferv4.p4
