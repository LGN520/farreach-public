#!/bin/bash
DIRNAME="netcache"

#set -e

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "clear tmp files"
rm tmp_switchos*.out
rm tmp_spineswitchos*.out
rm tmp_popserver*.out
rm tmp_cleaner*.out

echo "configure data plane"
cd leafswitch; bash configure.sh; cd ..
sleep 1s
cd spineswitch; bash configure.sh; cd ..
sleep 1s
cd clientrackswitch; bash configure.sh; cd ..
sleep 1s
echo "launch switchos"
mx switchos1 ./switchos 0 > tmp_switchos0.out &
mx switchos2 ./switchos 1 > tmp_switchos1.out &
mx switchos3 ./switchos 2 > tmp_switchos2.out &
mx switchos4 ./switchos 3 > tmp_switchos3.out &

mx spines1 ./switchos 0 spine > tmp_spineswitchos0.out &
mx spines2 ./switchos 1 spine > tmp_spineswitchos1.out &
mx spines3 ./switchos 2 spine > tmp_spineswitchos2.out &
# mx spines4 ./switchos 3 spine > tmp_spineswitchos3.out &

echo "launch ptfserver"
cd leafswitch; bash ptf_popserver.sh >../tmp_popserver_leaf.out 2>&1 &
sleep 1s
cd ..
cd spineswitch; bash ptf_popserver.sh >../tmp_popserver_spine.out 2>&1 &
sleep 1s
bash ptf_cachefrequencyserver.sh >../tmp_cachefrequencyserver.out 2>&1 &
cd ..
cd leafswitch; bash ptf_cleaner.sh >../tmp_cleaner.out 2>&1 &
sleep 1s
cd ..
