source ../common.sh

cd ../../common; make all; cd ../ycsb; bash compile.sh; cd ../$DIRNAME; make allclient
