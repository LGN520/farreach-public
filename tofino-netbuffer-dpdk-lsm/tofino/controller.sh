if [ $1 == "setup" ]
then
	python3 controller/controller.py >/dev/null 2>&1 &
	python3 controller/pull_listener.py >/dev/null 2>&1 &
	#python3 controller/controller.py &
	#python3 controller/pull_listener.py &
elif [ $1 == "cleanup" ]
then
	pids=$(ps -aux | grep "controller.py" | grep -v "grep" | awk '{print $2}')
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
	pids=$(ps -aux | grep "pull_listener.py" | grep -v "grep" | awk '{print $2}')
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
