set -x
#!/use/bin/env bash

keyword=$1

clientpids=( "$(ps -aux | grep "$1" | grep -v "localkill.sh" | grep -v "grep" | awk '{print $2}')" )

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
