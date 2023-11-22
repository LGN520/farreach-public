
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
rm tmp_switchos.out
rm tmp_popserver.out
rm tmp_snapshotserver.out
rm tmp_cleaner.out

echo "configure data plane"
cd bmv2; bash configure.sh; cd ..
sleep 1s

echo "launch ptfserver"
cd bmv2; 
mx switchos bash ptf_popserver.sh >../tmp_popserver.out 2>&1 &
sleep 1s
cd ..
cd bmv2; 
mx switchos bash ptf_snapshotserver.sh >../tmp_snapshotserver.out 2>&1 &
sleep 1s
cd ..
cd bmv2; 
mx switchos bash ptf_cleaner.sh >../tmp_cleaner.out 2>&1 &
sleep 1s
cd ..

if [ "x${recovermode}" == "xrecover" ]
then
	sleep 10s # wait for data plane interfaces UP; wait for ptf_popserver
	echo "launch switchos w/ recovery mode"
	mx switchos ./switchos ${recovermode} >tmp_switchos.out 2>&1 &
else
	echo "launch switchos"
	mx switchos ./switchos >tmp_switchos.out 2>&1 &
fi
