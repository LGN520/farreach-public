set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

cd common; make all; cd ..
cd $DIRNAME; make allserver; cd ..
