#!/bin/bash
DIRNAME="farreach"

#set -e

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

if [ $# -eq 1 ]
then
	recovermode=$1
	if [ "x${recovermode}" != "xrecover" ]
	then
		echo "[ERROR] incorrect arg: ${recovermode}"
		exit
	fi
else
	recovermode=""
fi

echo "clear tmp files"
rm tmp_switchos*.out
rm tmp_popserver.out
rm tmp_snapshotserver.out
rm tmp_cleaner.out

echo "configure data plane"
cd leafswitch; bash configure.sh; cd ..
sleep 1s
cd spineswitch; bash configure.sh; cd ..
sleep 1s
cd partitionswitch; bash configure.sh; cd ..
sleep 1s
echo "launch ptfserver"
cd leafswitch; 
bash ptf_popserver.sh >../tmp_popserver.out 2>&1 &
sleep 1s
cd ..

# cd leafswitch; 
# bash ptf_cleaner.sh >../tmp_cleaner.out 2>&1 &
# sleep 1s
# cd ..

echo "launch switchos"
mx switchos1 ./switchos 0 > tmp_switchos0.out &
mx switchos2 ./switchos 1 > tmp_switchos1.out &
mx switchos3 ./switchos 2 > tmp_switchos2.out &
mx switchos4 ./switchos 3 > tmp_switchos3.out &
mx switchos5 ./switchos 4 > tmp_switchos4.out &
mx switchos6 ./switchos 5 > tmp_switchos5.out &
mx switchos7 ./switchos 6 > tmp_switchos6.out &
mx switchos8 ./switchos 7 > tmp_switchos7.out &
# if [ "x${recovermode}" == "xrecover" ]
# then
# 	sleep 10s # wait for data plane interfaces UP; wait for ptf_popserver
# 	echo "launch switchos w/ recovery mode"
# 	mx switchos ./switchos ${recovermode} >tmp_switchos.out 2>&1 &
# else
# 	echo "launch switchos"
# 	mx switchos ./switchos >tmp_switchos.out 2>&1 &
# fi
