set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

cd common; make all; cd ..
cd benchmark/inswitchcache-java-lib; bash compile.sh; cd ../../
cd benchmark/ycsb; bash compile.sh; cd ../../
cd $DIRNAME; make allclient; cd ..
