DIRNAME="nocache"

# NOTE: you need to launch spine/leaf switch data plane before running this script under su account

echo "configure data plane"
cd tofino; bash configure.sh; cd ..
