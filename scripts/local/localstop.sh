#!/use/bin/env bash

keyword=$1

clientpids=( "$(ps -aux | grep "${keyword}" | grep -v "grep" | awk '{print $2}')" )

if [ ${#clientpids[@]} -gt 0 ]
then
	for i in ${!clientpids[@]}
	do
		if [ "${clientpids[i]}x" != "x" ]
		then
			kill -15 ${clientpids[i]}
		fi
	done
fi
