# Tofino-based NetBuffer + DPDK-based XIndex with persistency + variable-length key-value pair (tofino-netbuffer-dpdk-lsm-varkv)

## Implementation log

- Copy from tofino-netbuffer-dpdk-lsm to tofino-netbuffer-dpdk-lsm-varkv
- Support 16B key
	+ Add p4src/key.p4 and p4src/valid.p4
	+ Modify basic.p4
	+ Modify configure/table_configure.py
	+ Modify debug/read_register.py
	+ Modify periodic_update/read_register.py
	+ Modify trigger_update/read_register.py
	+ Add key.h and key.c, modify client.c, localtest.c, server.c. prepare.c, Makefile

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

## Fixed issues
