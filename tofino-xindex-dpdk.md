# Tofino-based forwarding + DPDK-based XIndex (tofino-xindex-dpdk)

## Implementation log

- Try TLDK
	+ Too complex to deploy
- DPDK-based CS architecture without persistency
	+ P4 problem: bf_switchd: /root/bf-sde-8.9.1/pkgsrc/bf-drivers/bf_switchd/bf_switchd.c:302: bf_switchd_pd_lib_init: Assertion pd_init_fn != ((void *)0)' failed'
		* Solve: rm corresponding directory in $SDE/p4-build/ and re-compile
	+ DWN status of port in tofino
		* You must enable correct port and launch your dpdk program (invoking rte_eth_dev_start)
	+ No packet sent to tofino
		* Tofino must enable the correct port, otherwise dpdk cannot send the packet
	+ No packet is transmitted to dl32
		* Type of IPv4 is 0x0008 in host (0x0800 is big endian)
	+ After client sends a packet, rte_eth_stats_get returns stats with ipackets=1, while rte_eth_rx_burst always returns 0
		* Give rxconf when setuping RX queue
		* Use multiple mbufs (e.g., 32) when invoking rte_eth_rx_burst; using 1 mbuf cannot receive any packet
	+ Receive packets in client side
		* Fix a bug related with IP address in server side
	+ Simple test
		* Pass single-thread test of XIndex: GET, PUT, DEL, SCAN
		* TODO: error in multiple-thread of client side
- TODO: DPDK-based CS architecture with persistency

## How to run

## Fixed issues
