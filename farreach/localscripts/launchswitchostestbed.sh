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
fi

echo "clear tmp files"
rm tmp_switchos.out
rm tmp_popserver.out
rm tmp_snapshotserver.out
rm tmp_cleaner.out

echo "configure data plane"
cd tofino; bash configure.sh; cd ..

echo "launch switchos"
nohup ./switchos ${recovermode} >tmp_switchos.out 2>&1 &

echo "launch ptfserver"
cd tofino; nohup bash ptf_popserver.sh >../tmp_popserver.out 2>&1 &
cd ..
cd tofino; nohup bash ptf_snapshotserver.sh >../tmp_snapshotserver.out 2>&1 &
cd ..
cd tofino; nohup bash ptf_cleaner.sh >../tmp_cleaner.out 2>&1 &
cd ..
