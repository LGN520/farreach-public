DIRNAME="distnocache"

if [[ $# -ne 1 ]]
then
	echo "Usage: bash launchswitchostestbed.sh spine/leaf"
	exit 1
fi

role=$1

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "configure data plane"
cd tofino-${role}; bash configure.sh; cd ..
