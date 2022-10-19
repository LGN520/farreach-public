DIRNAME="distcache"

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
rm tmp_cleaner.out

echo "configure data plane"
cd tofino-${role}; bash configure.sh; cd ..

echo "launch switchos"
nohup ./switchos ${role} >tmp_switchos.out 2>&1 &

echo "launch ptfserver"
cd tofino-${role}; nohup bash ptf_popserver.sh >../tmp_popserver.out 2>&1 &
cd ..
cd tofino; nohup bash ptf_cleaner.sh >../tmp_cleaner.out 2>&1 &
cd ..
