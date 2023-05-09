#!/bin/bash

if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

sed -i "s/^#define SERVER_ROTATION/\/\/#define SERVER_ROTATION/g" common/helper.h
source scripts/remote/sync_file.sh common helper.h
