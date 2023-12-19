set -x
DIRNAME="netcache"

#set -e

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "clear tmp files"
rm tmp_switchos.out
rm tmp_popserver.out
rm tmp_cleaner.out

echo "configure data plane"
cd bmv2; bash configure.sh; cd ..
sleep 1s

echo "launch switchos"
mx switchos ./switchos >tmp_switchos.out 2>&1 &

echo "launch ptfserver"
cd bmv2; mx switchos bash ptf_popserver.sh >../tmp_popserver.out 2>&1 &
sleep 1s
cd ..
cd bmv2; mx switchos bash ptf_cleaner.sh >../tmp_cleaner.out 2>&1 &
sleep 1s
cd ..
