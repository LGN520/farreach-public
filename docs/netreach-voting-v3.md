# Tofino-based NetREACH (voting-based) + DPDK-based XIndex with persistence + variable-length key-value pair (netreach-voting-v3)

- Remove hash_tbl, try_res_tbl, using 4B for each unit of key instead of 2B -> reduce 3 stages

## Workflow (Being designed)

## In-switch overview

* Stage 0: keylolo, keylohi, keyhilo, keyhihi; (16B key -> get iscached), load_backup_tbl
* Stage 1: valid
* Stage 2: vote, latest (for inconsistency of populated/deleted data), seq (for packet loss of PUT), case 1, case 3
* Stage 3: lock, vallen
	- Case 1: PUT/DELREQ on switch for backup
	- Case 3: First PUTREQ on server for backup
	- Case 2: First eviction on switch for backup (since eviction is based on control plane now, controller can handle it)
* Stage 4-10: val1-val14; (96B to 112B value, 1 to 2 more stages for 128B value)
* Stage 11: port_forward_tbl
	- Mark PUTREQ_GS and PUTREQ_PS if isbackup=1 as case2
	- Clone GETRES_CU and PUTREQ_CU
		+ Original one to load evicted data as PUTREQ_GS/PS
		+ Cloned one to update value as newly populated data

## Concerns

- Why we need data-plane-based population?
	+ Time of taking effect from us to ns for PUTs (make little sens in real workloads)
	+ If we want to cope with packet loss for cache eviction, we still need rtt-level time (us)
