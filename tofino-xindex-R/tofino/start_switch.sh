#!/usr/bin/env bash

sed -e 's/TOFINO_SINGLE_DEVICE/xindex/g' <$SDE/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in > $SDE_INSTALL/share/p4/targets/tofino/xindex.conf
cd $SDE
./run_switchd.sh -p xindex
