
serverpids=( "$(ps -aux | grep "./server" | grep -v "grep" | awk '{print $2}')" )

if [ ${#serverpids[@]} -gt 0 ]
then
	for i in ${!serverpids[@]}
	do
		if [ "${serverpids[i]}x" != "x" ]
		then
			kill -9 ${serverpids[i]}
		fi
	done
fi
