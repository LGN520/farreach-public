source ../common.sh

cd ../../common; make all
cd ../benchmark/inswitchcache-java-lib; bash compile.sh
cd ../ycsb; bash compile.sh
cd ../../$DIRNAME; make allclient
