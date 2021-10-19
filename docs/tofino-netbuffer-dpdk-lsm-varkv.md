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
	+ Pass localtest and remote test
	+ NOTE: Tofino will drop all packets without Ethernet header!!!
	+ NOTE: DPDK port will drop all packets without correct ethernet address even if enable promisc mode!!!
- Support YCSB
	+ Read source code of YCSB to see how to generate the workload from the parameter file
		* Client::main -> ClientThread::run -> Workload::doTransaction(DB db)
			- The workload will generate operation including key and fields according to the parameter file (not measured in statistics)
				+ DiscreteGenerator operationchooser.nextString (generate operation)
				+ NumberGenerator keychooser.nextValue().intValue() -> CoreWorkload::buildKeyName() (generate key)
				+ NumberGenerator fieldlengthgenerator.nextValue().longValue() (get size of value fields) -> StringByteIterator/RandomByteIterator (generate value)
			- Then it performs the operation by the given DB abstraction layer
		* Client::main -> Measurements::getMeasurements -> Get a global/static Measurements instance
			- Measurements::measure(operation, latency) -> Measurements::getOpMeasurement -> OneMeasurement::measure
				+ CoreWorkload will measure ReadModifyWrite
				+ BDWrapper will measure each operation
			- Measurements::exportMeasurements -> OneMeasurement::exportMeasurements -> Call TextMeasurementsExporter::write
			to output the measurement statistics saved by OneMeasurement
	+ Prepare netbuffer.dat to set properties of workload
	+ Implement parser to parser YCSB workload (ycsb/parser.h, ycsb/parser.c)
	+ Split workload into different parts for different client threads (split_workload.c)
	+ Add ycsb_local_client.c for loading phase (running in server side)
	+ Add data_put in xindex.h, xindex_impl.h, xindex_root.h, xindex_root_impl.h, xindex_group.h, and xindex_group_impl.h
	+ Pass workload_name to xindex (xindex.h, xindex_impl.h, xindex_root.h, xindex_root_impl.h, xindex_group.h, and xindex_group_impl.h)
	+ Update helper.h to get the directory name or file name related with workload
	+ Add serialize_root and deserialize_root in xindex_root.h and xindex_root_impl.h
	+ Add open in xindex.h, xindex_impl.h, xindex_root.h, xindex_root_impl.h, xindex_group.h, and xindex_group_impl.h
	+ CHECKPOINT: copy the previous change to tofino-xindex-dpdk-lsm-varkv
- NOTE: if workload is not skewed, server will be overloaded

## How to run

- Microbenchmark
	- Prepare randomly-generated keys
		+ NOTE: we direclty use makefile to enable DPDK (to detet ports) without cmake
		+ `make all`
		+ `./prepare`
- YCSB
	- Prepare workload for loading or transaction phase
		+ For example:
		+ `./bin/ycsb.sh load basic -P workloads/workloada -P netbuffer.dat > workloada-load.out`
		+ `./bin/ycsb.sh run basic -P workloads/workloada -P netbuffer.dat > workloada-run.out`
		+ `./split_workload load workloada threadnum`
		+ `./split_workload run workloada threadnum`
	- `./ycsb_local_client -h threadnum -p workloada`
	- Directory structure
		+ Raw workload file: workloada-load.out, workloada-run.out
		+ Split workload file: e.g., workloada-load-5/2.out
		+ Database directory: e.g., /tmp/netbuffer/workloada/group0.db, /tmp/netbuffer/workloada/buffer0-0.db
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
