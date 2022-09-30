#!/usr/bin/env bash

cd $SDE/pkgsrc/p4-build/
./configure --prefix=$SDE_INSTALL --with-tofino P4_NAME=netbufferv2 P4_PATH=/home/ssy/NetBuffer/netreach-voting-v2/tofino/basic.p4  enable_thrift=yes  P4_VERSION=p4-14
make
make install
sed -e 's/TOFINO_SINGLE_DEVICE/netbufferv2/g' <$SDE/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in > $SDE_INSTALL/share/p4/targets/tofino/netbufferv2.conf
