source ../common.sh

cd ../../common; make all; cd ../benchmark/ycsb; bash compile.sh; cd ../../$DIRNAME; make allclient
