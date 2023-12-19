
switchospids=( "$(ps -aux | grep "./switchos" | grep -v "grep" | awk '{print $2}')" )

if [ ${#switchospids[@]} -gt 0 ]
then
	for i in ${!switchospids[@]}
	do
		if [ "${switchospids[i]}x" != "x" ]
		then
			kill -15 ${switchospids[i]}
		fi
	done
fi
