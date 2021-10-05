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
- Fix a bug in rocksdb-model-v2 (table/block_based/block.cc, ModelSeek)
	+ For 8B key, IndexBlock::key does not have seq (aka user key); while for 16B key, IndexBlock::key has seq (aka internal key)
	+ raw_key_.IsUserKey comes fom index_includes_seq, so we need to identify it even for the key in index block
- Fix a bug in tofino (tofino/basic.p4)
	+ Get original valid bit and clone a packet of DEL_REQ_S to server only if the original bit is 1
- Fix a bug of rocksdb (cannot find after put)
	+ Reason: slice is shallow copy, we cannot give a pointer of local variable in Key::to_slice
- Support variable-length value
	+ Add tofino/basic.p4.120B.bak, tofino/p4src/header.p4.120B.bak, tofino/p4src/parser.p4.120B.bak, tofino/p4src/regs/key.p4.120B.bak,
	val.p4.120B.bak, and valid.p4.120B.bak
	+ Add val.h and val.c
	+ Remove trigger_update and periodic_update
	+ Add tofino/reportkv/read_register.py
- TODO: if workload is not skewed, server will be overloaded

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
