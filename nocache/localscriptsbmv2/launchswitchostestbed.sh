DIRNAME="nocache"

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "configure data plane"
cd bmv2; bash configure.sh; cd ..
sleep 1s
