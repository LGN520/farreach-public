if [ ${is_common_included} -ne 1 ]
then
	source scripts/common.sh
fi

cd common; make all; cd ..
cd benchmark/inswitchcache-java-lib; bash compile.sh; cd ../../
cd benchmark/ycsb; bash compile.sh; cd ../../
cd $DIRNAME; make allclient; cd ..
