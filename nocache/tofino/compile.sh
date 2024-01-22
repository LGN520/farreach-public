#!/usr/bin/env bash
cd ../../
source scripts/common.sh
cd nocache/tofino

# # cd $SDE/pkgsrc/p4-build/
# # # NOTE: for tofino compiler 9.4.0, main.p4 in P4_PATH must have the same name as P4_NAME

# # ./configure --prefix=$SDE_INSTALL --with-tofino --with-pd P4_NAME=nocache P4_PATH=${SWITCH_ROOTPATH}/nocache/tofino/nocache.p4  enable_thrift=yes  P4_VERSION=p4-16
# # make
# # make install
# # # # sed -e 's/TOFINO_SINGLE_DEVICE/nocache/g' <$SDE/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in > $SDE_INSTALL/share/p4/targets/tofino/nocache.conf
# # # # 
$SDE/p4_build.sh --with-tofino ${SWITCH_ROOTPATH}/nocache/tofino/nocache_16.p4 enable_thrift=yes

# $SDE/pkgsrc/p4-build/configure --with-tofino  --with-p4c=p4c --prefix=$SDE_INSTALL \
#       --bindir=$SDE_INSTALL/bin \
#       P4_NAME=nocache_16 \
#       P4_PATH=${SWITCH_ROOTPATH}/nocache/tofino/nocache_16.p4   \
#       P4_VERSION=p4-16 P4_ARCHITECTURE=tna \
#       LDFLAGS="-L$SDE_INSTALL/lib"
#       enable_thrift=yes
# make
# make install