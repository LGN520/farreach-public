set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

#

if [ $# -ne 0 ]
then
	echo "Usage: bash scriptsbmv2/remote/keydump_and_sync.sh"
	echo "Example: bash scriptsbmv2/remote/keydump_and_sync.sh"
	exit
fi

tmpworkloadname=$(readini keydump/config.ini "global" "workload_name")

echo "Run keydump for workload: ${tmpworkloadname}"
cd benchmark/ycsb
# Generate hottest/nearhot/coldest keys; calculate bottleneck serveridx; pre-generate workloads for server rotation under static pattern
if [ "x${tmpworkloadname}" == "xworkload-load" ]
then
	python2 ./bin/ycsb load keydump
else
	python2 ./bin/ycsb run keydump
fi
# Generate key populairty change rules for dynamic pattern
python generate_dynamicrules.py ${tmpworkloadname}
cd ../../

echo "Sync keydump files for ${tmpworkloadname} to clients/servers..."

