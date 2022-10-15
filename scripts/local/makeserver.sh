if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

cd common; make all; cd ..
cd $DIRNAME; make allserver; cd ..
