if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

set -x

cd ${DIRNAME}

# NOTE: you need to launch per-switch data plane and control plane before running this script

if [ "x${DIRNAME}" == "xnocache" ] || [ "x${DIRNAME}" == "xdistnocache" ]
then
	echo "clear tmp files in remote servers"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out"
elif [ "x${DIRNAME}" == "xfarreach" ]
then
	echo "clear tmp files in remote servers and controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out; rm tmp_reflector.out; rm tmp_controller.out; rm tmp_controller_bwcost.out"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out"
elif [ "x${DIRNAME}" == "xnetcache" ]
then
	echo "clear tmp files in remote servers and controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out; rm tmp_reflector.out; rm tmp_controller.out"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out"
elif [ "x${DIRNAME}" == "xdistcache" ]
then
	echo "clear tmp files in remote servers"
	rm tmp_reflector.out
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out; rm tmp_reflector.out; rm tmp_controller.out"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out"
elif [ "x${DIRNAME}" == "xdistfarreach" ]
then
	echo "clear tmp files in remote servers and controller"
	sudo rm tmp_reflector.out
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out; rm tmp_reflector.out; rm tmp_controller.out; rm tmp_controller_bwcost.out"
	ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; rm tmp_server.out"
fi

cd ..
source scripts/remote/stopservertestbed.sh
cd ${DIRNAME}

#echo "clear system cache"
#ssh -t ${USER}@${SERVER0} "echo 123456 | sudo -S sh -c 'echo 3 > /proc/sys/vm/drop_caches'; echo 123456 | sudo -S sh -c 'echo 4 > /proc/sys/vm/drop_caches'"
#ssh -t ${USER}@${SERVER1} "echo 123456 | sudo -S sh -c 'echo 3 > /proc/sys/vm/drop_caches'; echo 123456 | sudo -S sh -c 'echo 4 > /proc/sys/vm/drop_caches'"

if [ "x${DIRNAME}" == "xfarreach" ] || [ "x${DIRNAME}" == "xnetcache" ] || [ "x${DIRNAME}" == "xdistcache" ] || [ "x${DIRNAME}" == "xdistfarreach" ]
then
	echo "launch controller"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./controller >tmp_controller.out 2>&1 &"
fi

echo "launch servers"
ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 0 >tmp_server.out 2>&1 &"
ssh ${USER}@${SERVER1} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./server 1 >tmp_server.out 2>&1 &"

if [ "x${DIRNAME}" == "xdistcache" ] || [ "x${DIRNAME}" == "xdistfarreach" ]
then
	echo "launch reflectors"
	ssh ${USER}@${SERVER0} "cd ${SERVER_ROOTPATH}/${DIRNAME}; nohup ./reflector leaf >tmp_reflector.out 2>&1 &"
	sudo nohup ./reflector spine >tmp_reflector.out 2>&1 &
fi

cd ..
