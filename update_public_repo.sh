set -x

if [ $# -ne 1 ]
then
	echo "Usage: bash update_public_repo.sh <public_repo_link>"
	exit
fi

publicrepo=$1

# Get link for private repo, and show the latest commit
#privaterepo=$(git config --get remote.origin.url)
git show --summary

# Get last commit ID
lastcommitid=$(git show --summary | head -n 1 | awk '{print $2}')

# Backup submodule git files
mkdir ../tmp-farreach
mv .gitmodules ../tmp-farreach
mv benchmark/.git ../tmp-farreach/benchmark.git
mv benchmarkdist/.git ../tmp-farreach/benchmarkdist.git

# Remove submodule information
mv benchmark benchmark-tmp
mv benchmarkdist benchmarkdist-tmp
git submodule deinit benchmark
git submodule deinit benchmarkdist
git rm --cached benchmark
git rm --cached benchmarkdist
rm -r benchmark
rm -r benchmarkdist
mv benchmark-tmp benchmark
mv benchmarkdist-tmp benchmarkdist
git add benchmark
git add benchmarkdist
git add -f benchmark/output/.placeholder
git add -f benchmarkdist/output/.placeholder

# Add link for publich repo, and remove link for remote repo
git remote add publicrepo ${publicrepo}

# Remove unnecessary files from private repo (commented files are moved to ./deprecated or ./*/deprecated)
rm -r ./deprecated
rm -r ./docs
rm -r ./farreach/deprecated
rm -r ./netcache/deprecated
rm -r ./nocache/deprecated
rm evaluation-progress.md
rm rmhistory.sh
rm update_public_repo.sh
rm ycsb-implementation.md

# Update projects/farreach-private/ as projects/distreach/
tmpfiles=($(find nocache/ distnocache/ netcache/ distcache/ farreach/ distreach/ common/ scripts/ scriptsbmv2/ scriptsdist/ benchmark/ -type f -name "*.sh" -o -name "*.c" -o -name "*.h" | xargs grep -r -e "farreach-private/" -e "/farreach-private" -l | grep -v "update_publich_repo.sh"))
echo "${tmpfiles}"
# In Linux
echo "${tmpfiles}" | xargs sed -i 's!/farreach-private!/distreach!g'
echo "${tmpfiles}" | xargs sed -i 's!farreach-private/!distreach/!g'
# In MacOS
#echo "${tmpfiles}" | xargs sed -i '' 's!/farreach-private!/distreach!g'
#echo "${tmpfiles}" | xargs sed -i '' 's!farreach-private/!distreach/!g'

# Commit
git commit -am 're-organize for public repo'

# Push to public repo
git push -f publicrepo master

# Remove link for public repo
git remote remove publicrepo

# Reset private repo
git reset --hard ${lastcommitid}

# Resume submodule git files
mv ../tmp-farreach/.gitmodules ./
mv ../tmp-farreach/benchmark.git benchmark/.git
mv ../tmp-farreach/benchmarkdist.git benchmarkdist/.git
rm -r ../tmp-farreach

# Resume submodule
git submodule init
git submodule update
cd benchmark
git checkout master
git reset --hard HEAD
cd ..
cd benchmarkdist
git checkout master
git reset --hard HEAD
cd ..

# Show the latest commit for double-check, which should be the same at the beginning of the shell
git show --summary
