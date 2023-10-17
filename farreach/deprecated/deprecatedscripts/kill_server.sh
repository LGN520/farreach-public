#!/use/bin/env bash

pids=$(ps -aux | grep "./server" | grep -v "grep" | awk '{print $2}')

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
