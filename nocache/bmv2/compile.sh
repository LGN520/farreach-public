set -x
# #!/usr/bin/env bash
# cd ../../
# source scriptsbmv2/common.sh
# cd nocache/tofino

# cd $SDE/pkgsrc/p4-build/
# # NOTE: for tofino compiler 9.4.0, main.p4 in P4_PATH must have the same name as P4_NAME
# ./configure --prefix=$SDE_INSTALL --with-tofino P4_NAME=nocache P4_PATH=${SWITCH_ROOTPATH}/nocache/tofino/nocache.p4  enable_thrift=yes  P4_VERSION=p4-14
# make
# make install
# sed -e 's/TOFINO_SINGLE_DEVICE/nocache/g' <$SDE/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in > $SDE_INSTALL/share/p4/targets/tofino/nocache.conf
