if [ ${is_common_included} -ne 1 ]
then
	source scripts/common.sh
fi

cd common; make all; cd ..
cd $DIRNAME; make allswitch; cd ..
