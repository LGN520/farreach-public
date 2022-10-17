if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

#set -x

if [[ with_controller -eq 1 ]]
then
	# NOTE: if w/ in-switch cache, finish warmup phase by launching servers of correpsonding method + warmup_client + stopping servers
	echo "[part 0] pre-admit hot keys into switch before server rotation"

	echo "launch storage servers of ${DIRNAME}"
	source scripts/remote/launchservertestebed.sh
	sleep 10s

	echo "pre-admit hot keys"
	cd ${DIRNAME}
	./warmup_client
	cd ..
	sleep 10s

	echo "stop storage servers of ${DIRNAME}"
	source scripts/remote/stopservertestbed.sh
fi
