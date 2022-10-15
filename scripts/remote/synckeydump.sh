if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

function syncfiles_toclient() {
	TMPDIRNAME=$1

	ssh ${USER}@${SECONDARY_CLIENT} "rm -rf ${CLIENT_ROOTPATH}/$TMPDIRNAME"

	echo "sync to ${SECONDARY_CLIENT}"
	# NOTE: not --exclude "*.out" for benchmark/output/*
	rsync -av -e ssh --exclude "*.a" --exclude "*.bak" --exclude "*.o" --exclude "*.d" --exclude "*.html" ./$TMPDIRNAME ${USER}@${SECONDARY_CLIENT}:${CLIENT_ROOTPATH} >/dev/null
}

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/remote/synckeydump.sh <workloadname>"
	echo "Example: bash scripts/remote/synckeydump.sh workloada"
	exit
fi

tmpworkloadname=$1

syncfiles_toclient benchmark/output/${tmpworkloadname}-hotest.out
syncfiles_toclient benchmark/output/${tmpworkloadname}-coldest.out
syncfiles_toclient benchmark/output/${tmpworkloadname}-nearhot.out
syncfiles_toclient benchmark/output/${tmpworkloadname}-hotinrules
syncfiles_toclient benchmark/output/${tmpworkloadname}-hotoutrules
syncfiles_toclient benchmark/output/${tmpworkloadname}-randomrules
syncfiles_toclient benchmark/output/${tmpworkloadname}-pregeneration
