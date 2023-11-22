
DIRNAME="farreach"

echo "stop switchos"
bash localscriptsbmv2/stop_switchos.sh >/dev/null 2>&1

echo "stop ptfserver"
bash localscriptsbmv2/stop_ptfserver.sh >/dev/null 2>&1

echo "stop switch"
bash localscriptsbmv2/stop_switch.sh >/dev/null 2>&1
