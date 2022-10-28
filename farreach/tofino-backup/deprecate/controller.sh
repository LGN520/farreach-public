if [ $1 == "setup" ]
then
	python3 controller/periodic_update.py >/dev/null 2>&1 &
elif [ $1 == "cleanup" ]
then
	pids=$(ps -aux | grep "periodic_update.py" | grep -v "grep" | awk '{print $2}')
	if [ ${#pids[@]} -gt 0 ]
	then
		for i in ${!pids[@]}
		do
			if [ "${pids[i]}x" != "x" ]
			then
				kill -15 ${pids[i]}
			fi
		done
	fi
	pids=$(ps -aux | grep "trigger_update.py" | grep -v "grep" | awk '{print $2}')
	if [ ${#pids[@]} -gt 0 ]
	then
		for i in ${!pids[@]}
		do
			if [ "${pids[i]}x" != "x" ]
			then
				kill -15 ${pids[i]}
			fi
		done
	fi
fi
