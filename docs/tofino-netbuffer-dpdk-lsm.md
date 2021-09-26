# Tofino-based NetBuffer + DPDK-based XIndex with persistency (tofino-netbuffer-dpdk-lsm)

## Implementation log

- Copy from tofino-xindex-dpdk-lsm to tofino-netbuffer-dpdk-lsm
- Add PUT_REQ_S (server.c, packet_format.h, packet_format_impl.h)
- Add DEL_REQ_S (server.c, packet_format.h, packet_format_impl.h)
- Clone pkt for DEL (tofino/basic.c, tofino/configure/table_configure.py)
- Add message queue and optimize mbuf allocation (server.c)
- Add CBF (cbf.h, cbf_impl.h)
- Integrate CBF into NetBuffer (GET/PUT/DEL/free/compact) (xindex_group.h, xindex_group_impl.h)
- Implement backup mechanism (tofino/backup.sh, tofino/backup/read_register.py, tofino/controller.py, server.c)

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

## Fixed issues
