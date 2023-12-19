
#!/use/bin/env bash

clientpids=( "$(ps -aux | grep "./controller" | grep -v "grep" | awk '{print $2}')" )

if [ ${#clientpids[@]} -gt 0 ]
then
	for i in ${!clientpids[@]}
	do
		if [ "${clientpids[i]}x" != "x" ]
		then
			kill -9 ${clientpids[i]}
		fi
	done
fi
