if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/local/configure_server.sh <server_physical_idx>"
	exit
else
	server_physical_idx=$1
	echo "Configure physical server ${server_physical_idx} for ${DIRNAME}"

	# TODO: you need to change the following configuration according to your own testbed
	# NOTE: we use node1/master as two physical clients, and slave01/slave02/stresstest1/stresstest as four physical servers
	if [ ${server_physical_idx} -eq 0 ]
	then
		sudo ifconfig enp59s0f0 10.0.1.3/24
		sudo arp -s 10.0.1.1 b8:ce:f6:9a:02:56
		sudo arp -s 10.0.1.2 b8:ce:f6:99:fe:06
		sudo arp -s 10.0.1.3 b8:ce:f6:99:fb:0e
		sudo arp -s 10.0.1.4 b8:ce:f6:9a:00:6e
		sudo arp -s 10.0.1.5 08:c0:eb:24:6e:52
		sudo arp -s 10.0.1.6 b8:ce:f6:e8:f5:3e
		if [ ${is_distributed} -eq 1 ]
		then
			# bf2/bf3 as spine
			#sudo ifconfig enp129s0f0 10.0.2.16/24
			#sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78
			#sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9
			#sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58
			#sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0
		fi
	elif [ ${server_physical_idx} -eq 1 ]
	then
		sudo ifconfig enp59s0f0 10.0.1.4/24
		sudo arp -s 10.0.1.1 b8:ce:f6:9a:02:56
		sudo arp -s 10.0.1.2 b8:ce:f6:99:fe:06
		sudo arp -s 10.0.1.3 b8:ce:f6:99:fb:0e
		sudo arp -s 10.0.1.4 b8:ce:f6:9a:00:6e
		sudo arp -s 10.0.1.5 08:c0:eb:24:6e:52
		sudo arp -s 10.0.1.6 b8:ce:f6:e8:f5:3e
		if [ ${is_distributed} -eq 1 ]
		then
			# bf2/bf3 as spine
			#sudo ifconfig enp129s0f1 10.0.2.13/24
			#sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78
			#sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9
			#sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58
			#sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0
		fi
	elif [ ${server_physical_idx} -eq 2 ]
	then
		sudo ifconfig enp134s0f0 10.0.1.5/24
		sudo arp -s 10.0.1.1 b8:ce:f6:9a:02:56
		sudo arp -s 10.0.1.2 b8:ce:f6:99:fe:06
		sudo arp -s 10.0.1.3 b8:ce:f6:99:fb:0e
		sudo arp -s 10.0.1.4 b8:ce:f6:9a:00:6e
		sudo arp -s 10.0.1.5 08:c0:eb:24:6e:52
		sudo arp -s 10.0.1.6 b8:ce:f6:e8:f5:3e
		if [ ${is_distributed} -eq 1 ]
		then
			# bf2/bf3 as spine
			#sudo ifconfig enp129s0f1 10.0.2.13/24
			#sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78
			#sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9
			#sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58
			#sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0
		fi
	elif [ ${server_physical_idx} -eq 3 ]
	then
		sudo ifconfig enp134s0f0 10.0.1.6/24
		sudo arp -s 10.0.1.1 b8:ce:f6:9a:02:56
		sudo arp -s 10.0.1.2 b8:ce:f6:99:fe:06
		sudo arp -s 10.0.1.3 b8:ce:f6:99:fb:0e
		sudo arp -s 10.0.1.4 b8:ce:f6:9a:00:6e
		sudo arp -s 10.0.1.5 08:c0:eb:24:6e:52
		sudo arp -s 10.0.1.6 b8:ce:f6:e8:f5:3e
		if [ ${is_distributed} -eq 1 ]
		then
			# bf2/bf3 as spine
			#sudo ifconfig enp129s0f1 10.0.2.13/24
			#sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78
			#sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9
			#sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58
			#sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0
		fi
	fi

	sudo sysctl -w net.core.rmem_max=16777216
	sudo sysctl -w net.core.rmem_default=212992

	sudo hugeadm --thp-never

	#sudo sysctl -w vm.dirty_background_ratio=5
	#sudo sysctl -w vm.dirty_ratio=40

	# Only work for current shell process (inherited by subprocess) -> so we need to source the script
	ulimit -n 1024000
	# Display
	ulimit -n
fi
