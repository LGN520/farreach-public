DIRNAME="distfarreach"

#set -e

if [[ $# -ne 1 ]]
then
	echo "Usage: bash launchswitchostestbed.sh spine/leaf"
	exit 1
fi

role=$1

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "clear tmp files"
rm tmp_switchos.out
rm tmp_popserver.out
rm tmp_snapshotserver.out
rm tmp_cleaner.out

#echo "clear system cache"
#sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
#sudo sh -c 'echo 4 > /proc/sys/vm/drop_caches'

echo "configure data plane"
cd tofino-${role}; bash configure.sh; cd ..

echo "launch switchos"
nohup ./switchos ${role} >tmp_switchos.out 2>&1 &

echo "launch ptfserver"
cd tofino-${role}; nohup bash ptf_popserver.sh >tmp_popserver.out 2>&1 &
cd ..
cd tofino-${role}; nohup bash ptf_snapshotserver.sh >tmp_snapshotserver.out 2>&1 &
cd ..
cd tofino-${role}; nohup bash ptf_cleaner.sh >tmp_cleaner.out 2>&1 &
cd ..
