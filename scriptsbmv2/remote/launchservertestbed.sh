set -x
if [ "x${is_common_included}" != "x1" ]
then
	source scriptsbmv2/common.sh
fi

#

if [ $# -eq 1 ]
then
	recovermode=$1
	if [ "x${recovermode}" != "xrecover" ]
	then
		echo "[ERROR] incorrect arg: ${recovermode}"
		exit
	fi
else
	recovermode=""
fi

cd ${DIRNAME}

# NOTE: you need to launch per-switch data plane and control plane before running this script

echo "clear tmp files in remote servers and controller if any"
cd ${SERVER_ROOTPATH}/${DIRNAME};
rm tmp_server0.out; rm tmp_reflector.out; rm tmp_controller.out; rm tmp_controller_bwcost.out
rm tmp_server1.out

if [ ${with_controller} -eq 1 ]
then
	echo "launch controller"
	cd ${SERVER_ROOTPATH}/${DIRNAME}
	mx h3 ./controller >tmp_controller.out 2>&1 &
fi

echo "launch servers"
cd ${SERVER_ROOTPATH}/${DIRNAME}
mx h3 ./server 0 >tmp_server0.out ${recovermode} 2>&1 &
mx h4 ./server 1 >tmp_server1.out ${recovermode} 2>&1 &

if [ ${with_reflector} -eq 1 ]
then
	echo "launch reflectors"
	mx h3 ./reflector leaf >tmp_reflector.out 2>&1 &
	mx h1 ./reflector spine >tmp_reflector.out 2>&1 &
	cd ..
fi

cd ..