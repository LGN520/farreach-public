# Tofino-based forwarding + DPDK-based XIndex with persistency (tofino-xindex-dpdk-lsm)

## Implementation log

- Install RocksDB 6.22.1

## How to run

- Prepare randomly-generated keys
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
