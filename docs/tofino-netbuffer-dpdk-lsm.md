# Tofino-based NetBuffer + DPDK-based XIndex with persistency (tofino-netbuffer-dpdk-lsm)

## Implementation log

- Copy from tofino-xindex-dpdk-lsm to tofino-netbuffer-dpdk-lsm
- Add PUT_REQ_S (server.c, packet_format.h, packet_format_impl.h)
- Add DEL_REQ_S (server.c, packet_format.h, packet_format_impl.h)
- Clone pkt for DEL (tofino/basic.c, tofino/configure/table_configure.py)
- Add message queue and optimize mbuf allocation (server.c) (SYNC to tofino-xindex-dpdk-lsm)
- Add CBF (cbf.h, cbf_impl.h)
- Integrate CBF into NetBuffer (GET/PUT/DEL/free/compact) (xindex_group.h, xindex_group_impl.h)
- Implement backup mechanism (tofino/backup.sh, tofino/backup/read_register.py, tofino/controller.py, server.c)
- Add CPU port mechanism (tofino/cpuport/, tofino/cpuport.sh)
	+ Set CPU port fail: cannot get packet in OS
	+ Set copy_to_cpu fail: cannot get packet in OS
	+ Solution
		* Server pulls KV from switch OS after receving scan, we calculate latency for our original design (server.c, tofino/pull_listerner.py)
			- XIndex SCAN: client -> P4 switch -> server -> XIndex SCAN
			- NetBuffer SCAN: client -> P4 switch
				+ -> server -> SCAN + wait + Merge (if wait >= 0); SCAN + Merge (wait < 0)
				+ -> controller (P4 switch OS) -> get KV -> normal switch -> server -> update KV
					* We assume that the ports between controller and server are DPDK ports
					* The latency from P4 switch to switch OS, and that within normal switch are ns-level
				+ Approximately, wait time = getKV + updateKV - SCAN
		* We can compile P4 with setting copy_to_cpu to get the hardware resource usage
- Support SCAN with KV listener and backuper
	+ tofino/controller/periodic_update.py -> periodic_update/read_register.py -> backuper in server.c: periodically send KV to server for backup
	+ tofino/controller/trigger_update.py -> trigger_update/read_register.py -> listener in server.c: trigger KV update after receiving pull request for SCAN

## How to run

- Prepare randomly-generated keys
	+ NOTE: we direclty use makefile to enable DPDK (to detet ports) without cmake
	+ `make all`
	+ `./prepare`
- Run `cd tofino`
	+ Run `su` to enter root account
	+ Run `bash compile.sh` to compile p4 into binary code
	+ Run `bash start_switch.sh` to launch Tofino
	+ Create a new terminal and run `bash configure.sh` to configure data plane
- Run `bash start_server.sh` in server host
- Run `bash start_client.sh` in client host
- Switch
	+ `cd tofino`
	+ `bash controller.sh setup`
	+ END: `bash controller.sh cleanup`
	+ Legacy: `python cpuport/recv.py`

## Fixed issues
