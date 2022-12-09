if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

function syncfiles_toclient() {
	TMPDIRNAME=$1
	TMPFILENAME=$2

	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $CLIENT_ROOTPATH/$TMPDIRNAME"

	echo "sync ${TMPDIRNAME}/${TMPFILENAME} to ${SECONDARY_CLIENT}"
	# NOTE: not --exclude "*.out" for benchmark/output/*
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/${TMPDIRNAME} >/dev/null
}

function syncfiles_toall(){
	TMPDIRNAME=$1
	TMPFILENAME=$2

	ssh ${USER}@${LEAFSWITCH} "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@bf3 "rm -rf ${SWITCH_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SWITCH_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $CLIENT_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SERVER0} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"
	ssh ${USER}@${SERVER1} "rm -rf ${SERVER_ROOTPATH}/$TMPDIRNAME/$TMPFILENAME; mkdir -p $SERVER_ROOTPATH/$TMPDIRNAME"

	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${LEAFSWITCH}"
	# NOTE: not --exclude "*.out" for benchmark/output/*
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${LEAFSWITCH}:${SWITCH_ROOTPATH}/${TMPDIRNAME} >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to bf3"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@bf3:${SWITCH_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SECONDARY_CLIENT}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SERVER0}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER0}:${SERVER_ROOTPATH}/$TMPDIRNAME >/dev/null
	echo "sync ${TMPDIRNAME}/$TMPFILENAME to ${SERVER1}"
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" $TMPDIRNAME/$TMPFILENAME ${USER}@${SERVER1}:${SERVER_ROOTPATH}/$TMPDIRNAME >/dev/null
}

if [ $# -ne 0 ]
then
	echo "Usage: bash scripts/remote/keydump_and_sync.sh"
	echo "Example: bash scripts/remote/keydump_and_sync.sh"
	exit
fi

tmpworkloadname=$(readini keydump/config.ini "global" "workload_name")

echo "Run keydump for workload: ${tmpworkloadname}"
cd benchmark/ycsb
# Generate hottest/nearhot/coldest keys; calculate bottleneck serveridx; pre-generate workloads for server rotation under static pattern
./bin/ycsb run keydump
# Generate key populairty change rules for dynamic pattern
python generate_dynamicrules.py ${tmpworkloadname}
cd ../../

echo "Sync keydump files for ${tmpworkloadname} to clients/servers..."
syncfiles_toclient benchmark/output ${tmpworkloadname}-hotest.out
syncfiles_toclient benchmark/output ${tmpworkloadname}-coldest.out
syncfiles_toclient benchmark/output ${tmpworkloadname}-nearhot.out
syncfiles_toclient benchmark/output/${tmpworkloadname}-hotinrules \*
syncfiles_toclient benchmark/output/${tmpworkloadname}-hotoutrules \*
syncfiles_toclient benchmark/output/${tmpworkloadname}-randomrules \*
syncfiles_toclient benchmark/output/${tmpworkloadname}-pregeneration \*

# sync hotkey files to servers for netcache/distcache to resume cached keyset under server rotation
syncfiles_toall benchmark/output \*-hotest.out
