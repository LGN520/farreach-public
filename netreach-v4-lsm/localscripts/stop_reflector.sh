#!/use/bin/env bash

reflectorpids=( "$(ps -aux | grep "./reflector" | grep -v "grep" | awk '{print $2}')" )

if [ ${#reflectorpids[@]} -gt 0 ]
then
	for i in ${!reflectorpids[@]}
	do
		if [ "${reflectorpids[i]}x" != "x" ]
		then
			kill -15 ${reflectorpids[i]}
		fi
	done
fi
