filename=$1

diff -y --suppress-common-lines farreach/$filename nocache/$filename > diffnocache.out
diff -y --suppress-common-lines farreach/$filename netcache/$filename > diffnetcache.out
diff -y --suppress-common-lines farreach/$filename distnocache/$filename > diffdistnocache.out
diff -y --suppress-common-lines farreach/$filename distcache/$filename > diffdistcache.out
diff -y --suppress-common-lines farreach/$filename distfarreach/$filename > diffdistfarreach.out
