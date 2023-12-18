ptf_popserverpids=( "$(ps -aux | grep "ptf_popserver" | grep -v "grep" | awk '{print $2}')" )

if [ ${#ptf_popserverpids[@]} -gt 0 ]
then
	for i in ${!ptf_popserverpids[@]}
	do
		if [ "${ptf_popserverpids[i]}x" != "x" ]
		then
			kill  ${ptf_popserverpids[i]}
		fi
	done
fi

ptf_cleanerpids=( "$(ps -aux | grep "ptf_cleaner" | grep -v "grep" | awk '{print $2}')" )

if [ ${#ptf_cleanerpids[@]} -gt 0 ]
then
	for i in ${!ptf_cleanerpids[@]}
	do
		if [ "${ptf_cleanerpids[i]}x" != "x" ]
		then
			kill  ${ptf_cleanerpids[i]}
		fi
	done
fi
