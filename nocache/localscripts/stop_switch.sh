switchpids=( $(ps -aux | grep "switch" | grep -v "grep" | awk '{print $2}') )

if [ ${#switchpids[@]} -gt 0 ]
then
	for i in ${!switchpids[@]}
	do
		if [ "${switchpids[i]}x" != "x" ]
		then
			kill -15 ${switchpids[i]}
		fi
	done
fi
