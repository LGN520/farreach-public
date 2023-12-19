
DIRNAME="distcache"

echo "stop switchos"
bash localscripts/stop_switchos.sh >/dev/null 2>&1

echo "stop ptfserver"
bash localscripts/stop_ptfserver.sh >/dev/null 2>&1

echo "stop switch"
bash localscripts/stop_switch.sh >/dev/null 2>&1
