DIRNAME="netcache"

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "clear tmp files"
rm tmp_switchos.out
rm tmp_popserver.out
rm tmp_cleaner.out

echo "configure data plane"
bash tofino/configure.sh

echo "launch switchos"
nohup ./switchos >tmp_switchos.out 2>&1 &

echo "launch ptfserver"
nohup bash tofino/ptf_popserver.sh >tmp_popserver.out 2>&1 &
nohup bash tofino/ptf_cleaner.sh >tmp_cleaner.out 2>&1 &
