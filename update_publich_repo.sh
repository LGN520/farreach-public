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
mv benchmark/.git ../tmp-farreach

# Remove submodule information
mv benchmark benchmark-tmp
git submodule deinit benchmark
git rm --cached benchmark
rm -r benchmark
mv benchmark-tmp benchmark
git add benchmark
git add -f benchmark/output/.placeholder

# Add link for publich repo, and remove link for remote repo
git remote add publicrepo ${publicrepo}

# Remove unnecessary files from private repo (commented files are moved to ./deprecated or ./*/deprecated)
#rm -r ./common/reserved_files
rm -r ./deprecated
#rm -r ./distcache
#rm -r ./distfarreach
#rm -r ./distnocache
#rm -r ./failedtrials
#rm -r ./futuretrials
#rm -r ./ovs
#rm -r ./tommyds-2.2
#rm ./scripts/local/calculate_statistics_helper.py.bak.*
#rm -r ./scripts/remote/deprecated
rm -r ./docs
rm -r ./farreach/deprecated
#rm -r ./farreach/deprecated-src
#rm -r ./farreach/deprecated-synthetic
#rm -r ./farreach/deprecated-tofino
#rm -r ./farreach/deprecatedscripts
#rm -r ./farreach/tofino/deprecate
#rm -r ./farreach/workloadparser
rm -r ./netcache/deprecated
#rm -r ./netcache/deprecated-src
#rm -r ./netcache/deprecated-synthetic
#rm -r ./netcache/deprecated-tofino
#rm -r ./netcache/deprecatedscripts
rm -r ./nocache/deprecated
#rm -r ./nocache/deprecated-src
#rm -r ./nocache/deprecated-synthetic
#rm -r ./nocache/deprecated-tofino
#rm -r ./nocache/deprecatedscripts
rm evaluation-progress.md
rm rmhistory.sh
rm update_public_repo.sh
rm ycsb-implementation.md

## Rename benchmark.md
#mv benchmark.md README.md
#git add README.md

# Update projects/farreach-private/ as projects/farreach-public/
tmpfiles=($(find nocache/ netcache/ farreach/ common/ scripts/ benchmark/ -type f -name "*.sh" -o -name "*.c" -o -name "*.h" | xargs grep -r -e "farreach-private/" -e "/farreach-private" -l | grep -v "update_publich_repo.sh"))
echo "${tmpfiles}"
# In Linux
echo "${tmpfiles}" | xargs sed -i 's!/farreach-private!/farreach-public!g'
echo "${tmpfiles}" | xargs sed -i 's!farreach-private/!farreach-public/!g'
# In MacOS
#echo "${tmpfiles}" | xargs sed -i '' 's!/farreach-private!/farreach-public!g'
#echo "${tmpfiles}" | xargs sed -i '' 's!farreach-private/!farreach-public/!g'

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
mv ../tmp-farreach/.git benchmark/
rm -r ../tmp-farreach

# Resume submodule
git submodule init
git submodule update
cd benchmark
git checkout master
git reset --hard HEAD
cd ..

# Show the latest commit for double-check, which should be the same at the beginning of the shell
git show --summary
