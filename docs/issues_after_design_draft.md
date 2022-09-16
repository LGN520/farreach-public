# Issues found after writing design draft

## Preliminary results

+ The raw data of each method is saved in NetBuffer/method/results/preliminary/
- Preliminary results of write-only workload
	+ Static workload (hash partition w/ 1024 clients + 128 servers)
		* farreach: 20.22 MOPS thpt, 105.94 us avg latency
		* nocache: 1.45 MOPS thpt, 1377 us avg latency
			- NOTE: much larger avg latency due to server-side processing and queuing lateny
		* netcache: 0.17 MOPS thpt, 12855 us avg latency -> 0.91 MOPS thpt, 1841.82 us avg latency
			- Reason: extremely low thpt & large latency due to waiting for server-issued in-switch value update ACKs for cache coherence
				+ NOTE: thpt of nocache is 8X of netcache in our testbed, while thpt of nocache is 2X of netcache in netcache paper
			- Solution: NOT wait for ACK in netcache/distcache now, as they may argue that pktloss is rare and it is an engineering issue
			- Result: slightly worse than nocache which matches the results in netcache paper
		* distnocache: 1.17 MOPS thpt, 1396 us avg latency
		* distcache: 1.24 MOPS thpt, 1796 us avg latency
		* distfarreach: 18.5 MOPS thps, 107 us avg latency
	+ Dynamic workload (hash partition w/ 1024 clients + 8 servers)
		* Write-only workload
			- DONE: farreach, nocache, netcache, distnocache, distcache, distfarreach
		* Read-only workload
			- DONE: farreach, nocache, netcache, distnocache, distcache, distfarreach
		* 50%-write-50%-read workload
			- DONE: farreach, nocache, netcache, distnocache, distcache, distfarreach

## During prepare preliminary results

+ Under read-only dynamic workload, farreach is worse than netcache
	* Observation: not related with snapshot; not related with waiting for CACHE_POP_ACK; not related with server-side snapshot in rocksdb
	* Reason: different rocksdb files
	* [IMPORTANT] Solution: we should keep the same rocksdb files before evaluating each method
		- NOTE: still slightly smaller than NetCache due to limited conservative reads

+ DistCache/DistFarreach cannot populate new hot keys efficiently
	* Reason: ptf_cleaner fails after removing cm4
	+ [IMPORTANT] Optimize P4 code to support 4 CM register arrays in distcache and distfarreach
		* NOTE: poor perf under dynamic workload is due to the crashed ptf_cleaner when resetting cm4 \[and bf3\] instead of decreased CM register array (actually hash_calc/hash_calc1 and hash_calc2/hash_calc3 have the same hashed result)
		* TODOTODO: If we have sth new to implement in switch. we can consider to remove cm2 and cm4 to save hardware resources -> NOTE: update ptf_cleaner accordingly!!!

## About two design issues: recirculate for single pipeline mode and read-after-write-consistency for rare case with packet loss

- Single pipeline mode
	+ Under our Tofino, recirculate does not support cross-ingress-pipeline switching
		* The latest Tofino already supports it by recirculate_odd_pipe
- Read-after-write consistency
	+ In the rare case with packet loss, client may get a stale value before an ACKed value of the key being evicted -> violate key-value store semantics
+ [IMPORTANT] Discuss with Qun about single pipeline mode and read-after-write-consistency
	* For single switch pipeline
		- From design: recirculate issue is due to the limitation of Tofino itself, which is supported by PISA or Jupiter -> not the issue of our design
		- From evaluation: as Tofino data plane can update the snapshot module after receiving the command from the switch OS within limited time, we do not observe inconsistent results in our evaluation
	* For read-after-write-consistency (or availability)
		- For request-based invalidation: violate client-side semantics
		- For response-based invalidation: introduce a new problem about how to define a latest record
		- For request-based invalidation + in-switch sequence-based packet value update: too complex for programmable switch
		- Solution: request-based invalidation + server-based limited blocking

## Implementation for the above two design issues

### Read blocking for read-after-write consistency (availability)

### hardware/software-link-based recirculation for single pipeline mode

## About open issues

- Adaptiveness
	+ [DEPRECATED]: Tune CM threshold for distcache and distfarreach due to reducing one CM register array to fix Tofino resource limitation
		* NOTE: we can use CM theoretical guarantee on error bound to argue that why we increase CM threshold if decrease CM hashnum by 1
		* NOTE: we periodically reset CM such that N/w is close to one, set a proper phi for single switch scenario, and then phi' = (phi)^(4/3)*(W/N)^(3/4) for the same error probability under distributed scenario
			- NOTE: although we round phi to set CM threshold for single switch, we should use the original phi to calcualte phi' to keep the same error probability before rounding phi'
			- Example of N/W=1: for sigma=(1/[100/150/200])^4, d=4, threshold=100/150/200 -> d=3, threshold=100^(4/3)=500/800/1200
			- Example of N/W=1.25 (W=64K, N=80KOPS for requests accessing storage servers): for sigma=(1/124)^4, d=4, threshold=125 -> d=3, threshold=125^(4/3)*(1.25)^(3/4)=800 -> we set threshold = 100 and 1000 after rounding
	+ Soft reset CM (reset to a threshold instead of to zero)
		* NOTE: CM warmup after reset is not an issue under our scenario
