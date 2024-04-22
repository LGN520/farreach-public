#!/bin/bash
DIRNAME="nocache"

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "configure data plane"
cd leafswitch; bash configure.sh; cd ..
sleep 1s

cd spineswitch; bash configure.sh; cd ..
sleep 1s