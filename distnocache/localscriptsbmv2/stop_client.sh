set -x
#!/use/bin/env bash

clientpids=( "$(ps -aux | grep "./remote_client" | grep -v "grep" | awk '{print $2}')" )

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
