set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

#

echo "stop servers"
# bash scriptsbmv2/local/localstop.sh ./server >/dev/null 2>&1

if [ ${with_controller} -eq 1 ]
then
	echo "stop controller"
	bash scriptsbmv2/local/localstop.sh ./controller >/dev/null 2>&1
fi

if [ ${with_reflector} -eq 1 ]
then
	echo "stop reflectors"
	bash scriptsbmv2/local/localstop.sh ./reflector >/dev/null 2>&1
fi

sleep 15s # wait for database to finish flush and compaction

echo "kill servers"
bash scriptsbmv2/local/localkill.sh ./server >/dev/null 2>&1

if [ ${with_controller} -eq 1 ]
then
	echo "kill controller"
	bash scriptsbmv2/local/localkill.sh ./controller >/dev/null 2>&1
fi

if [ ${with_reflector} -eq 1 ]
then
	echo "kill reflector"
	bash scriptsbmv2/local/localkill.sh ./reflector >/dev/null 2>&1
fi
