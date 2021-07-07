#!/usr/bin/env bash

cd $SDE/pkgsrc/p4-build/
./configure --prefix=$SDE_INSTALL --with-tofino P4_NAME=xindex P4_PATH=/home/ssy/NetBuffer/tofino-xindex-R/tofino/basic.p4  enable_thrift=yes  P4_VERSION=p4-14
make
make install
