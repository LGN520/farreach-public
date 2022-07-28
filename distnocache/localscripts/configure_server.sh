if [ $# -ne 1 ]
then
	echo "Usage: bash configure_server.sh server_physical_idx"
else
	server_physical_idx=$1

	# TODO: you need to change the following configuration according to your own testbed
	# NOTE: we use dl11/dl15 as two physical clients, and dl16/dl13 as two physical servers
	if [ ${server_physical_idx} -eq 0 ]
	then
		sudo ifconfig enp129s0f1 10.0.1.16/24
		sudo arp -s 10.0.1.11 3c:fd:fe:bb:ca:79
		sudo arp -s 10.0.1.13 3c:fd:fe:bb:c9:c8
		sudo arp -s 10.0.1.15 3c:fd:fe:b5:28:59
		sudo arp -s 10.0.1.16 3c:fd:fe:b5:1f:e1
		# bf2 as spine
		sudo ifconfig enp129s0f0 10.0.2.16/24
		sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78
		sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9
		sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58
		sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0
	elif [ ${server_physical_idx} -eq 1 ]
	then
		sudo ifconfig enp129s0f0 10.0.1.13/24
		sudo arp -s 10.0.1.11 3c:fd:fe:bb:ca:79
		sudo arp -s 10.0.1.13 3c:fd:fe:bb:c9:c8
		sudo arp -s 10.0.1.15 3c:fd:fe:b5:28:59
		sudo arp -s 10.0.1.16 3c:fd:fe:b5:1f:e1
		# bf2 as spine
		sudo ifconfig enp129s0f1 10.0.2.13/24
		sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78
		sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9
		sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58
		sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0
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
