DIRNAME="distfarreach"

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

echo "configure data plane"
bash tofino-${role}/configure.sh

echo "launch switchos"
nohup ./switchos ${role} >tmp_switchos.out 2>&1 &

echo "launch ptfserver"
nohup bash tofino-${role}/ptf_popserver.sh >tmp_popserver.out 2>&1 &
nohup bash tofino-${role}/ptf_snapshotserver.sh >tmp_snapshotserver.out 2>&1 &
nohup bash tofino-${role}/ptf_cleaner.sh >tmp_cleaner.out 2>&1 &
