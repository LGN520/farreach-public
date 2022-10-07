# README for benchmark (running guide)

- method: farreach / nocache / netcache / distfarreach / distnocache / distcache

## Important NOTEs

- If code changes in ../common (including packet_format.\* and socket_helper.\*)
	+ SYNC to inswitchcache-java/jnisrc/site_ycsb_SocketHelper.c

## Environment installation

- Install boost
	+ `sudo apt-get install libboost-all-dev`
- Install bpftools for spine reflector
	+ `git clone https://github.com/cloudflare/bpftools.git`
	+ `cd bpftools`
	+ `make`
- Disable THP to reduce write_memtable_time in rocksdb
	+ `sudo apt-get install hugepages`
	+ `sudo hugeadm --thp-never`
	+ `cat /sys/kernel/mm/transparent_hugepage/enabled` to check

## Code compilation

- Under main client (i.e., the first physical client dl11), enter the project root directory
	+ `bash sync.sh` to sync source code to all machines
	+ NOTE: you need to change sync.sh based on your own testbed
- Under each server (NOT including client/switch), compile RocksDB
	+ `sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev`
	+ `cd rocksdb-6.22.1`
	+ `PORTABLE=1 make static_lib`
		* Comment -Wstrict-prototype and -Werror if compilation fails due to strict-alias warning
		* Add PORTABLE=1 if with runtime error of illegal instruction when open()
		* Possible error: compression type Snappy is not linked with the binary
			- Install libsnappy and remake static lib of rocksdb
			- Prepare the directory for database in advance
- Compile source code
	+ Compile software code
		* Manual way
			- Under project root directory of each client, run ``
		* Automatic way: under main client (i.e., the first physical client dl11), enter directory of method/
			- `bash remotescripts/makeremotefiles` to make C/C++ and java code including client, switchos, controller, and server
	+ Compile hardware code
		* (NOT need to run now) Under each Tofino OS, enter directory of method/tofino/ (or method/tofino-spine/ and method/tofino-leaf/)
			- `bash compile.sh` (NOTE: if we have already compiled for all methods, we do NOT need to run this command unless we change in-switch implementation)

## Normal running with a given workload X

- Under the main client, enter directory of ycsb/
	+ Get hot/cold keys for static/dynamic workloads, and get bottleneck partition for server rotation
		* `./bin/ycsb run keydump -P <workloadpattern> -pi 0 -df <workloadname>`
		* For example, `./bin/ycsb run keydump -P workload/workloada -pi 0 -df workloada`
	+ Generate rulemap rule files for dynamic workload
		* `python generate_dynamicrules.py <workloadname>`
		* For example, `python generate_dynamicrules.py workloada`
