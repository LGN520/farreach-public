# UDP-based NoCache + LSM-based KVS (rocskdb) + NO in-switch cache

- Copy from netreach-v4-lsm
- See [important notes](./netreach-v4-lsm.md) in netreach-v4-lsm.md
+ IMPORTANT NOTE for optype enumeration
	- We keep the same optype enumeration fo FarReach, NoCache, and NetCache (files: packet_format.h, tofino/main.p4, tofino/common.py, tofino/p4src/parser.p4)
	- When adding a new optype
		+ If it is only processed by end-hosts, you need to update optype enumeration and packet format implementation (files: packet_format*, tofino/main.p4, tofino/common.py, tofino/p4src/parser.p4)
		+ If it is processed by switch, besides previous files, you also need to consider ingress partition_tbl + ig_port_forward_tbl, and egress eg_port_forward_tbl + update_pktlen_tbl + update_ipmac_srcport_tbl
	- NOTE: packet implemention is different (e.g., NetCache has inswitch_hdr.hashval_for_bfX and clone_hdr.server_sid/port yet FarReach NOT need)

## Overview

- Client
	+ Keep the same as in netreach-v4-lsm
- Switch
	+ Keep key-based partition and forwarding MAT
- Server
	+ Remove cache pop/eviction and snapshot from server
- Remove controller and switchos

## Implementation log

+ Implemet NoCache
	* Client: not change
	* Switch: only provide basic forwarding (range/hash partition)
		- Replace "netbufferv4" into "nocache" in tofino/
		- Remove ptf_popserver/snapshotserver/cleaner and recover_switch
		- Packet format (p4src/header.p4, p4src/parser.p4)
			+ Remove all headers and parser except those for ethernet, ipv4, udp, op_hdr, scan_hdr, split_hdr, and meta fields for RANGE_SUPPORT
				+ Remove inswitch_hdr and parse_inswitch
				+ Remove meta.need_recirculate/cmX_predicate/is_hot/is_latest/is_deleted/access_val_mode/is_case1/is_lastclone_for_pktloss
				+ Remove seq_hdr and parse_seq
				+ Remove frequency_hdr and parse_frequency
				+ Remove validvalue_hdr and parse_validvalue
				+ Remove clone_hdr.client_udpport, and update CLONE_BYTES=1 in packet_format.h
				+ Remove vallen_hdr and valX_hdr, and parse_vallen and parse_valX
				+ Remove clone_hdr.clonenum_for_pktloss and parse_clone
				+ Remove stat_hdr and parse_stat
				+ Remove shadowtype_hdr and parse_shadowtype
		- Ingress pipeline (nocache.p4, p4src/ingress_mat.p4)
			+ Remove KV/CM/SEQ_BUCKET_COUNT, DEFAULT_HH_THRESHOLD, LOOKUP_ENTRY_COUNT
			+ Remove need_recirculate_tbl, set_hot_threshold_tbl, recirculate_tbl
			+ Move hash_for_partition_tbl, range_partition_tbl, and hash_partition_tbl one stage backward
			+ Remove cache_lookup_tbl, hash_for_cmX_tbl, hash_for_seq_tbl, hash_calcX, snapshot_flag_tbl, prepare_for_cachehit_tbl
			+ Move ipv4_forward_tbl two stages backward, and remove the action of forward_speical_get_response()
			+ Remove sample_tbl
			+ Move ig_port_forward_tbl two stages backward, and remove inswitchhdr-related actions (only keep the action for SCANREQ)
		- Egress pipeline (nocache.p4, p4src/egress_mat.p4)
			+ Remove p4src/regs/* and all corresponding MATs
				+ Remove p4src/regs/cm.p4 and access_cmX_tbl
				+ Remove p4src/regs/seq.p4, access_seq_tbl, access_savedseq_tbl
				+ Remove p4src/regs/cache_frequency.p4 and access_cache_frequency_tbl	
				+ Remove p4src/regs/validvalue.p4 and access_validvalue_tbl
				+ Remove p4src/regs/latest.p4 and access_latest_tbl
				+ Remove p4src/regs/deleted.p4 and access_deleted_tbl
				+ Remove p4src/regs/val.p4, update_vallen_tbl, and update_vallo/hiX_tbl
				+ Remove p4src/regs/case1.p4 and access_case1_tbl
			+ Update process_scanreq_split_hdr to remove clonehdr fields, and move into stage 0
			+ Remove is_hot_tbl
			+ Remove save_client_udpport_tbl
			+ Change lastclone_lastscansplit_tbl/counter to lastscansplit_tbl/counter, move into stage 1, and only for RANGE_SUPPORT
			+ Remove inswitchhdr/conservativeread/cachepop/case1/cacheevict/setvalid-related actions from eg_port_forward_tbl, move into stage 2, and only for RANGE_SUPPORT
			+ Remove action of update_ipmac_srcport_switch2switchos() from update_ipmac_srcport_tbl, move into stage 3
			+ Update update_pktlen_tbl only for RANGE_SUPPORT and SCANREQ_SPLIT, and move into stage 4
			+ Remove add_and_remove_value_header_tbl and drop_tbl
		- Configuration: configure/table_configure.py
			+ NOTE: update MAT size accordingly
			+ Ingress pipeline
				* NOTE: remove meta.need_recirculate for all MATs
				* Remove need_recirculate_tbl, set_hot_threshold_tbl, recirculate_tbl, cache_lookup_tbl, hash_for_cmX_tbl, hash_for_seq_tbl, snapshot_flag_tbl, preprare_for_cachehit_tbl, sample_tbl
				* Keep GETREQ/PUTREQ/DELREQ/LOADREQ for hash_for_partition_tbl, hash_partition_tbl
				* Keep GETREQ/PUTREQ/DELREQ/SCANREQ/LOADREQ for range_partition_tbl
				* Keep GETRES/PUTRES/DELRES/SCANRES_SPLIT/LOADACK for ipv4_forward_tbl
				* Keep SCANREQ for ig_port_forward_tbl
			+ Egress pipeline
				* Remove access_cmX_tbl, is_hot_tbl, access_cache_frequency_tbl, access_validvalue_tbl, access_seq_tbl, save_client_udpport_tbl, access_latest_tbl, access_deleted_tbl, update_vallen_tbl, access_savedseq_tbl, access_case1_tbl
				* Remove update_valX_tbl and the function of configure_update_val_tbl()
				* Remove add_and_remove_value_header_tbl, drop_tbl
				* Keep SCANREQ_SPLIT for lastscansplit_tbl (remove match field of clone_hdr.clonenum_for_pktloss)
				* Remove the function of configure_eg_port_forward_tbl()
				* Keep SCANREQ_SPLIT for configure_eg_port_forward_tbl_with_range()
					- Keep match fields of op_hdr.optype, meta.is_last_scansplit, meta.server_sid for eg_port_forward_tbl
				* Keep SCANREQ_SPLIT for update_pktlen_tbl (remove match field of vallen_hdr.vallen)
				* Update update_ipmac_srcport_tbl
					- Keep GETRES/PUTRES/DELRES/SCANRES_SPLIT/LOADACK for server2client (i.e., remove WARMUPAK)
					- Keep GETREQ/PUTREQ/DELREQ/SCANREQ_SPLIT/LOADREQ for client2server
					- Remove switch2switchos
	* Server: only keep server.worker
		- Remove reflector.popserver/worker (server.c, reflector_impl.h)
		- Remove server.popclient/evictserver/snapshotserver/snapshotdataserver (server.c, server_impl.h)
		- Only keep GETREQ/PUTREQ/DELREQ/SCANREQ/LOADREQ, and remove seq (server_impl.h)
		- Only keep rocksdb's operation (remove DeletedSet, snapshot, and seq), and convert val from/to string for rocksdb without seq (rocksdb_wrapper.*)
		- Remove server.loadfinishserver (server.c)
	* Remove warmup_client, loadfinish_client, controller, switchos, and recover/* (loadfinish_client.c warmup_client.c, controller.c, switchos.c, Makefile)
+ Debug NoCache
	* Pass GETREQ
	* Pass PUTREQ
	* Pass DELREQ
	* Pass LOADREQ
	* Pass SCANREQ
+ Test NoCache under static workload (server rotation) and dynamic workload
	* Dynacmic workload
		- Runtime throughput is ~0.9 MOPS in most time, but with performance degradation to 0.6~0.7 MOPS
		- Normalized throughput keeps ~10 without fluctuation
	* Server rotation under static workload
		- Aggregate system throughput: 1.27 MOPS
		- per-server throughput: min 0.09 MOPS, max 0.1 MOPS, avg 0.11 MOPS
+ Update netreach-v4-lsm and nocache
	* Add L2/L3 routing (if op_hdr.valid == false) at the beginning of ingress pipeline to be compatible with traditional network protocols (files: l2l3_forward_tbl in netbufferv4.p4, p4src/ingress_mat.md, configure/table_configure.py) -> sync to NoCache
		- NOTE: FarReach/NoCache packet will reset egress port by partition_tbl
	* Make FarReach compatible with traditional network protocols
		- NOTE: in design, we only need to reserve one udp port of worker_port_start; but in simulation, we need to match multiple udp ports
			* Simulate phyiscal server by server thread -> need to match worker_port_start + logical_server_idx -> if with one thread per physical server, we can only use worker_port_start
			* Simulate hardware link from switch to switchos by reflector, which is co-located with server -> need to match reflector_dp2cp/cp2dpserver_port -> if with hardware link, we can launch switchos.dpwcpserver as reflector.dp2cp/cp2dpserver at worker_port_start
				- switchos.dpwcpserver can listen on worker_port_start to receive packets from data plane (udp.dstport)
				- switchos.popworker/snapshotserver can send packet to switchos.dpwcpserver by message queue, which sends packet to data plane futher (udp.srcport)
		- Match udp.src/dstport with [worker_port_start, worker_port_start + max_logical_server_num] and reflector_dp2cp/cp2dpserver_port in parser (files: config.ini, p4src/parser.p4, configs/*) -> sync to NoCache
			* Match udp.dstport for worker ports -> support packets from client to server
			* Match udp.srcport for worker ports -> support packets from server to client
			* Match udp.dstport for reflector.dp2cpserver port -> support packets from switch to switchos
			* Match udp.srcport for reflector.cp2dpserver port -> support packets from switchos to switch
		- For range query, we need to match split_hdr.globalserveridx in process_scanreqsplit_tbl -> sync to NoCache
			* Add split_hdr.globalserveridx and change SPLIT_PREV_BYTES accordingly (files: p4src/header.p4, packet_format.h)
			* For the first SCANREQ_SPLIT (files: p4src/ingress_mat.md, p4src/egress_mat.md, configure/table_configure.py)
				- Set split_hdr.globalserveridx as well as udp.dstport and eport in range_partition_tbl
				- Pass end_globalserveridx_plus_one to calculate split_hdr.max_scannum in range_partition_for_scan_endkey_tbl
				- Set meta.server_sid for next SCANREQ_SPLIT of split_hdr.globalserveridx + 1 in process_scanreq_split_tbl
				- Increase split_hdr.globalserveridx by 1 in eg_port_forward_tbl
			* For each cloned SCANREQ_SPLIT
				- Set udp.dstport for current SCANREQ_SPLIT of split_hdr.globalserveridx in process_scanreq_split_tbl
				- Set meta.server_sid for next SCANREQ_SPLIT of split_hdr.globalserveridx + 1 in process_scanreq_split_tbl
				- Increase split_hdr.globalserveridx by 1 in eg_port_forward_tbl
		- Each physical client assign client.udpport in [client_port_start, client_port_start + logical_client_num] (files: config.ini, iniparser/iniparser_wrapper.*, common_impl.h, remote_client.c) -> sync to NoCache
			* NOTE: client udpports do not overlap with server.worker udpports
		- Each physical server assign worker.udpport in [worker_port_start, worker_port_start + logical_server_num] (files: server_impl.h) -> sync to NoCache
	* Pass correctness test of range partition
		- Issue: stat_hdr.stat is always 0, even if we set stat_hdr.stat = 1 in data plane (e.g., PUTREQ_INSWITCH -> PUTRES for cache hit (but stat_hdr.nodeidx_for_eval = 0xFF is correct); or CACHE_EVICT_LOADDATA_INSWITCH -> ACK with action data of stat = 1)
			+ Trial: re-compile -> FAIL
			+ Reason: tofino p4 compiler mis-reuses PHV of stat_hdr.stat for other field
			+ Solution: add 8-bit padding at the end of stat_hdr, and update pktlen configuration -> OK
				* Files: p4src/header.p4, p4src/egress_mat.md, configure/table_configure.py, and packet_format.*
	* Pass correctness test of hash partition
		- Issue: unable to place eg_port_forward_tbl due to pragma constraint
			+ Trial: comment l2l3_forward_tbl -> FAIL
			+ Trial: directly parse_op after parse_udp -> FAIL
			+ Trial: comment split_hdr.globalserveridx increase from eg_port_forward_tbl -> FAIL
			+ Trial: change stage of eg_port_forward_tbl -> FAIL
			+ Trial: reduce size of eg_port_forward_tbl from 8192 to 4096 -> OK

## Run

- Hardware configure
	+ Configure after each login
		* `source configure_client.sh`: configure NIC ipv4 address, arp table, UDP rcvbuf size, and openfd limitation
		* `source configure_server.sh`: configure NIC ipv4 address, arp table, UDP rcvbuf size, and openfd limitation
		* `sudo bash configure_switchos.sh`: configure UDP rcvbuf size
	+ Max # of open files
		* `sudo vim /etc/security/limits.conf` to set hard and soft limits on maximum # of open files
		* logout and re-login
		* `ulimit -n number` to set soft # of open files
- Prepare synthetic Zipf workload for loading and transaction phase
	+ Modify synthetic-generator/common.py to configure max_key, max_hotkey, and query_num
		+ Update config.ini to configure correct workload name
	+ `python gen_kv.sh` to generate workload for loading phase
		+ `./split_workload load linenum` -> workloada-load-{split_num}/*-*.out
	+ `python gen_queries_zipf.sh` to generate Zipf workload for transaction phase
		+ `./split_workload run linenum` -> workloada-run-{server_num}/*.out
- Prepare YCSB workload for loading or transaction phase
	+ For example:
	+ `./bin/ycsb.sh load basic -P workloads/workloada -P netbuffer.dat > workloada-load.out`
	+ `./bin/ycsb.sh run basic -P workloads/workloada -P netbuffer.dat > workloada-run.out`
	+ `./split_workload load linenum` -> workloada-load-{split_num}/*-*.out
	+ `./split_workload run linenum` -> workloada-run-{server_num}/*.out
- Change partition method (hash/range partition)
	+ RANGE_SUPPORT in tofino/netbufferv4.p4
	+ USE_HASH/RANGE in helper.h
	+ RANGE_SUPPORT in tofino/common.py
- Local loading phase
	- `./loader` to launch loaders in end host
	- If using ycsb, after ycsb loading phase, run `./loadfinish_client` in each physical server
- Transaciton phase (NOTE: no warmup phase due to no in-switch cache)
	- Switch
		- Run `cd tofino`
		+ Run `su` to enter root account
		+ Run `bash compile.sh` to compile p4 into binary code
		+ Run `bash start_switch.sh` to launch Tofino
	- Configure switch (NOTE: not need switchos and ptf.popserver/snapshotserer/cleaner due to no in-switch cache)
		+ Create a new terminal and run `bash configure.sh` to configure data plane
	- NOTE: not need controller due to no in-switch cache
	- Launch servers in end host
		+ `./server server_physical_idx`
		+ NOTE: to close server, use `sudo kill -15` to send SIGKILL
	- Launch clients in end host (NOTE: not need warmup_client due to no in-switch cache)
		- Transaction phase: `./remote_client client_physical_idx`
- Server rotation for static workload
	- Use config.ini.rotation-switch-switchos-loading-warmupclient
		+ Start switch and configure data plane
		+ Start switchos and ptf.pop/snapshotserver
		+ Start clients and servers for loading phase
		+ Start clients and servers for warmup phase
	- Use config.ini.rotation-transaction
		+ Start clients and servers for transaction phase (repeat 127 times)
		+ Example of 128 server threads
			* 1 server physical num + 1 server total logical num
				- (1) set server0.server_logical_idxes as bottleneck server logical index in config,ini
				- (2) launch bottleneck server thread in server0
				- (3) Run `./remote_client 1` in client 1
				- (4) Run `./remote_client 0` in client 0
				- (5) Record client0's server rotation data of all physical clients into result.out
			* 2 server physical num + 2 server total logical num
				- (1) set server0.server_logical_idxes as bottleneck server logical index in config.ini
				- (2) Change server1.server_logical_idxes (not equal to server0.server_logical_idxes), e.g., from 0 to 1, in config.ini
				- (3) Run `bash test_server_rotation.sh` in client0
				- (4) Record client0's server rotation data of all physical clients into result.out
				- (5) Go back to step (1) until repeating 127 times
	- IMPORTANT: try different # of client threads to sature servers
		+ NOTE: more client threads does not mean better throughput, as client threads have CPU contention overhead
		+ We MUST try different # of client threads to get the best runtime thpt improvement
- Dynamic workload
	+ Use configs/config.ini.dl16dl13
- Utils scripts
	- Help to update config.ini
		+ gen_logical_idxes: generate server logical indexes from startidx to endidx
	- Help to generate throughput result files
		+ sum_tworows_for_bottleneckserver.py: sum over per-server pktcnts of two clients to find bottleneck partition
		+ sum_twofiles.py: sum over per-server results to get aggregate statistics (NOTE: the two files must have the same content format)
		+ Deprecated (covered by client0.rotationdataserver): gen_rotation_onerow_result.py: generate one row of rotation throughput result files by summing over per-client rotation result line 
	- Analyze throughput result files: dynamic/static/rotation_calculate_thpt.py
	- sync_file.sh: sync one file (filepath relateive to netreach-v4-lsm/) to all other machines
	- ../sync.sh: sync entire netreach-v4-lsm directory to other machines (NOTE: old directory of other machines will be deleted first)

## Simple test

- NOTE: set switch_kv_bucket_num, KV_BUCKET_COUNT, CM_BUCKET_COUNT, HH_THRESHOLD, SEQ_BUCKET_COUNT as 1 before test
- Test cases of normal operations: see directory of "testcases/normal"
	+ Case 1: single read (GETREQ arrives at server)
	+ Case 2: single write (PUTREQ arrives at server)
	+ Case 3: single delete (DELREQ arrives at server)
	+ Case 4: single load (LOADREQ arrives at server)
	+ Case 5: single scan (SCANREQ_SPLIT arrives at server)
- NOTE: no testcases for cache population/eviction/hit and crash-consistent snapshot

## Fixed issues

## Future work
