set -x
username=$1
tmpdir=$2

remoteserver="dl11"
remotepath="~/projects/farreach-private/benchmark/"

scp -r ./${tmpdir} ${username}@${remoteserver}:~

ssh ${username}@${remoteserver} "cd ./${tmpdir}; cp --parents -r ./* ${remotepath}; cd ..; rm -r ${tmpdir}"
