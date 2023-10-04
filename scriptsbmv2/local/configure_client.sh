if [ "x${is_common_included}" != "x1" ]
then
	source scripts/common.sh
fi

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/local/configure_client.sh <client_physical_idx>"
	exit
else
	client_physical_idx=$1
	echo "Configure physical client ${client_physical_idx} for ${DIRNAME}"

	# TODO: you need to change the following configuration according to your own testbed
	# NOTE: we use dl11/dl15 as two physical clients, and dl16/dl13 as two physical servers
	if [ ${client_physical_idx} -eq 0 ]
	then
		sudo ifconfig ${MAIN_CLIENT_TOSWITCH_IFNAME} ${MAIN_CLIENT_TOSWITCH_IP}/24
		sudo arp -s ${MAIN_CLIENT_TOSWITCH_IP} ${MAIN_CLIENT_TOSWITCH_MAC} -i ${MAIN_CLIENT_TOSWITCH_IFNAME}
		sudo arp -s ${SECONDARY_CLIENT_TOSWITCH_IP} ${SECONDARY_CLIENT_TOSWITCH_MAC} -i ${MAIN_CLIENT_TOSWITCH_IFNAME}
		sudo arp -s ${SERVER0_TOSWITCH_IP} ${SERVER0_TOSWITCH_MAC} -i ${MAIN_CLIENT_TOSWITCH_IFNAME}
		sudo arp -s ${SERVER1_TOSWITCH_IP} ${SERVER1_TOSWITCH_MAC} -i ${MAIN_CLIENT_TOSWITCH_IFNAME}
		if [ ${is_distributed} -eq 1 ]
		then
			# bf2/bf3 as spine
			#sudo ifconfig enp129s0f0 10.0.2.11/24
			#sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78 -i enp129s0f0
			#sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9 -i enp129s0f0
			#sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58 -i enp129s0f0
			#sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0 -i enp129s0f0
			echo "[ERROR] NO distributed testbed now!"
			exit 1
		fi
	elif [ ${client_physical_idx} -eq 1 ]
	then
		sudo ifconfig ${SECONDARY_CLIENT_TOSWITCH_IFNAME} ${SECONDARY_CLIENT_TOSWITCH_IP}/24
		sudo arp -s ${MAIN_CLIENT_TOSWITCH_IP} ${MAIN_CLIENT_TOSWITCH_MAC} -i ${SECONDARY_CLIENT_TOSWITCH_IFNAME}
		sudo arp -s ${SECONDARY_CLIENT_TOSWITCH_IP} ${SECONDARY_CLIENT_TOSWITCH_MAC} -i ${SECONDARY_CLIENT_TOSWITCH_IFNAME}
		sudo arp -s ${SERVER0_TOSWITCH_IP} ${SERVER0_TOSWITCH_MAC} -i ${SECONDARY_CLIENT_TOSWITCH_IFNAME}
		sudo arp -s ${SERVER1_TOSWITCH_IP} ${SERVER1_TOSWITCH_MAC} -i ${SECONDARY_CLIENT_TOSWITCH_IFNAME}
		if [ ${is_distributed} -eq 1 ]
		then
			# bf2/bf3 as spine
			#sudo ifconfig enp129s0f0 10.0.2.15/24
			#sudo arp -s 10.0.2.11 3c:fd:fe:bb:ca:78 -i enp129s0f0
			#sudo arp -s 10.0.2.13 3c:fd:fe:bb:c9:c9 -i enp129s0f0
			#sudo arp -s 10.0.2.15 3c:fd:fe:b5:28:58 -i enp129s0f0
			#sudo arp -s 10.0.2.16 3c:fd:fe:b5:1f:e0 -i enp129s0f0
			echo "[ERROR] NO distributed testbed now!"
			exit 1
		fi
	fi

	sudo sysctl -w net.core.rmem_max=16777216
	sudo sysctl -w net.core.rmem_default=212992

	# Only work for current shell process (inherited by subprocess) -> so we need to source the script
	ulimit -n 1024000
	# Display
	ulimit -n
fi
