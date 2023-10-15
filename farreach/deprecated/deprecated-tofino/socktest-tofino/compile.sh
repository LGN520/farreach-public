set -x
source ../../scripts/common.sh
#!/usr/bin/env bash

cd $SDE/pkgsrc/p4-build/
./configure --prefix=$SDE_INSTALL --with-tofino P4_NAME=socktest P4_PATH=${SWITCH_ROOTPATH}/netreach-v4/socktest-tofino/basic.p4  enable_thrift=yes  P4_VERSION=p4-14
make
make install
sed -e 's/TOFINO_SINGLE_DEVICE/socktest/g' <$SDE/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in > $SDE_INSTALL/share/p4/targets/tofino/socktest.conf
