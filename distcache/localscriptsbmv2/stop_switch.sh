switchpids=( "$(ps -aux | grep "python network.py" | grep -v "grep" | grep -v "stop_switch.sh" | grep -v "stopswitchtestbed.sh" | awk '{print $2}')" )
  
if [ ${#switchpids[@]} -gt 0 ]
then
	for i in ${!switchpids[@]}
	do
		if [ "${switchpids[i]}x" != "x" ]
		then
			kill ${switchpids[i]}
		fi
	done
fi
sleep 5s
mn -c

switchpids=( "$(ps -aux | grep "/usr/bin/ovs-testcontroller" | grep -v "grep" | grep -v "stop_switch.sh" | grep -v "stopswitchtestbed.sh" | awk '{print $2}')" )
  
if [ ${#switchpids[@]} -gt 0 ]
then
	for i in ${!switchpids[@]}
	do
		if [ "${switchpids[i]}x" != "x" ]
		then
			kill ${switchpids[i]}
		fi
	done
fi
# /usr/bin/ovs-testcontroller
switchpids=( "$(ps -aux | grep "simple_switch" | grep -v "grep" | grep -v "stop_switch.sh" | grep -v "stopswitchtestbed.sh" | awk '{print $2}')" )
  
if [ ${#switchpids[@]} -gt 0 ]
then
	for i in ${!switchpids[@]}
	do
		if [ "${switchpids[i]}x" != "x" ]
		then
			kill  ${switchpids[i]}
		fi
	done
fi
