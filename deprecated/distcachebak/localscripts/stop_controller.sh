
#!/use/bin/env bash

controllerpids=( "$(ps -aux | grep "./controller" | grep -v "grep" | awk '{print $2}')" )

if [ ${#controllerpids[@]} -gt 0 ]
then
	for i in ${!controllerpids[@]}
	do
		if [ "${controllerpids[i]}x" != "x" ]
		then
			kill -15 ${controllerpids[i]}
		fi
	done
fi
