set -x
username="ssy"
tmpdir="tmpsync"

filepaths=$(git status --porcelain | grep -E "M|A" | awk '{print $2}')
filepaths_array=("$filepaths")
mkdir ./${tmpdir}
for filepath in ${filepaths_array[@]}
do
	rsync -R ${filepath} ./${tmpdir}
done
cp syncfiles_fromdl1.sh ./${tmpdir}

scp -P 8804 -r ./${tmpdir} ${username}@47.105.117.154:~

ssh ${username}@47.105.117.154 -p 8804 "bash tmpsync/syncfiles_fromdl1.sh $username $tmpdir"

ssh ${username}@47.105.117.154 -p 8804 "rm -r ./${tmpdir}"
rm -r ./${tmpdir}
