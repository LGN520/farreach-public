# AE Instructions

Here are the detailed instructions to reproduce experiments in our paper.

# Table of Contents

1. [Artifact Claims](#1-artifact-claims)
2. [AE Testbed Overview](#2-ae-testbed-overview)
3. [Notes for Artifact Evaluation](#3-notes-for-artifact-evaluation)
4. [AE on YCSB Core Workloads](#4-ae-on-ycsb-core-workloads)
	1. [Throughput Analysis](#41-throughput-analysis)
	2. [Latency Analysis](#42-latency-analysis)
	3. [Scalability Analysis](#43-scalability-analysis)
5. [AE on Synthetic Workloads](#5-ae-on-synthetic-workloads)
	1. [Impact of Write Ratio](#51-impact-of-write-ratio)
	2. [Impact of Key Distribution](#52-impact-of-key-distribution)
	3. [Impact of Value Size](#53-impact-of-value-size)
	4. [Impact of Key Popularity Changes](#54-impact-of-key-popularity-changes)
6. [AE on Snapshot Generation and Crash Recovery](#6-ae-on-snapshot-generation-and-crash-recovery)
	1. [Performance of Snapshot Generation](#61-performance-of-snapshot-generation)
	2. [Crash Recovery Time](#62-crash-recovery-time)

## 1. Artifact Claims

- First, we claim that **the results of experiments in AE may be different from those in our paper**, as the AE testbed is different from our evaluation testbed (unavailable now), including but not limited to different OSs, different numbers of CPU cores, different memory sizes, different disk I/O capabilities, different network interfaces, and different software packages.
	- However, we emphasize that **the conclusions of our paper still hold**, i.e., FarReach can achieve larger throughput and smaller latency compared with the baselines (NoCache and Netcache).

</br>

- Second, we claim that **you may encouter some problem when executing scripts during AE** due to different reasons, e.g., testbed misconfiguration, script misusage, resource confliction (AE testbed is shared with other researchers instead of our exclusive, while Tofino switch data plane cannot support multiple P4 programs simultaneously), Tofino hardware bugs (front panel ports sometimes cannot be activated unless waiting for several minutes), and our code bugs (not found in evaluation).
	- However, we have tried our best to avoid such problems via executing scripts in AE testbed by ourselves in advance. If you still encouter any problem and need helps, you can contact us if available (sysheng21@cse.cuhk.edu.hk).

## 2. AE Testbed Overview

- Here are the machines in AE testbed.
	+ Four physical machines with Ubuntu 16.04 or Ubuntu 18.04:
		* One main client (hostname: dl11).
		* One secondary client (hostname: dl20).
		* Two servers (hostnames: dl21 and dl30).
			- Note: controller is co-located in the first server (dl21).
	+ One 2-pipeline Tofino switch with SDE 8.9.1 (hostname: bf3).

</br>

- Here are the network settings in AE testbed,
	+ All machines are in the same local area network (IP mask: 172.16.255.255; NOT bypass Tofino switch data plane):
		* Main client: 172.16.112.11
		* Secondary client: 172.16.112.20
		* First server (co-located with controller): 172.16.112.21
		* Second server: 172.16.112.30
		* Tofino switch OS: 172.16.112.19
	+ All machines are also in the same Tofino-based network (bypass Tofino switch data plane):
		* Main client (NIC: enp129s0f0; MAC: 3c:fd:fe:bb:ca:78) <-> Tofino switch (front panel port: 5/0).
		* Secondary client (NIC: ens2f0; MAC: 9c:69:b4:60:34:f4) <-> Tofino switch (front panel port: 21/0).
		* First server (NIC: ens2f1; MAC: 9c:69:b4:60:34:e1) <-> Tofino switch (front panel port: 6/0).
		* Second server (NIC: ens3f1; MAC: 9c:69:b4:60:ef:c1) <-> Tofino switch (front panel port: 3/0).
		* Tofino switch (front panel port: 7/0) <-> Tofino switch (front panel port: 12/0) (for in-switch cross-pipeline recirculation to fix atomicity issue of crash-consistent snapshot generation in FarReach).

## 3. Notes for Artifact Evaluation

- **Note: we have finished system preparation and data preparation, such that you can directly execute scripts to reproduce experiments.**
	- System preparation includes software dependency installation, testbed configuration settings (e.g., Linux username, involved machines, and SSH settings), and code compilation **(server rotation is enabled by default for most experiments)**.
	- Data preparation includes loading phase (pre-loading 100M records into server-side key-value storage) and workload analysis (calculate bottleneck partition by consistent hashing and generate workload files for server rotation).
	- Although we have configured `scripts/global.sh` based on AE testbed and the AE account (username: atc2023ae), **please also make a double-check on `scripts/global.sh` by yourself.**

</br>

- **Note: most experiments use server rotation to simulate a large scale of servers, which is time-consuming:**
	- Each iteration fixes the bottleneck partition (and deploys a non-bottleneck partition) (**TIME: around 5 minutes**).
	- Each server rotation comprises tens of iterations (16-128) to simulate multiple machines (**TIME: around 1.5-12 hours**).
	- Each experiment needs multiple server rotations for different methods and parameter settings (**TIME: around 1-3 day(s)**).
	- Each round includes multiple experiments to evaluate from different perspectives (**TIME: around 1-2 week(s)**).
	- We need multiple rounds to reduce runtime variation (**TIME: around 1-2 month(s)**).
- **To save time, you can follow the guides of each experiment later to select a part of methods and parameter settings that you want before executing the corresponding script.**

</br>

- **Please ensure that our code is correctly compiled before running each experiment.**
	- For most experiments, we need to compile code in all machines to enable server rotation if not. Here is the **detailed compilation to enable server rotation under main client (dl11)**:
		- Enable server rotation:
			- Uncomment line 82 (#define SERVER_ROTATION) in `common/helper.h` to enable server rotation.
			- Run `bash scripts/remote/sync_file.sh common helper.h` to sync the code change to all machines.
		- Re-compile code of FarReach:
			- Set `DIRNAME` as `farreach` in `scripts/common.sh`
			- Run `bash scripts/remote/sync_file.sh scripts common.sh`
			- Under main client (dl11) and secondary client (dl20), run `bash scripts/local/makeclient.sh`
			- Under first server (dl21) and second server (dl30), run `bash scripts/local/makeserver.sh`
			- Under Tofino switch (bf3), run `bash scripts/local/makeswitchos.sh`
		- Re-compile code of NoCache and NetCache similar as FarReach:
			- The only difference is to set `DIRNAME` as `nocache` and `netcache`, respectively.
	- For the two experiments on impact of key popularity changes and performance of snapshot generation, we need to compile code in all machines to disable server rotation if not. Here is the **detailed compilation to disable server rotation under main client (dl11)**:
		- Disable server rotation:
			- Comment line 82 (//#define SERVER_ROTATION) in `common/helper.h` to enable server rotation.
			- Run `bash scripts/remote/sync_file.sh common helper.h` to sync the code change to all machines.
		- Re-compile code of FarReach, NoCache, and NetCache with the same steps as mentioned above.

## 4. AE on YCSB Core Workloads

### 4.1 Throughput Analysis

- **Pre-requisite: code is compiled with enabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_throughput.sh`:
	- You can keep a part of methods in `exp1_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp1_core_workload_list` to save time (default value is `"workloada" "workloadb" "workloadc" "workloadd" " workladf" "workload-load"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_throughput.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_throughput.sh 0 >tmp_exp_throughput.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any
$ awk -v flag=0 'flag == 0 && /\[exp1\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_throughput.out

# Get throughput results of NoCache if any
$ awk -v flag=0 'flag == 0 && /\[exp1\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_throughput.out

# Get throughput results of NetCache if any
$ awk -v flag=0 'flag == 0 && /\[exp1\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_throughput.out
```

### 4.2 Latency Analysis

- **Pre-requisite: code is compiled with enabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_latency.sh`:
	- You can keep a part of methods in `exp2_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of target throughput values `exp2_target_thpt_list` to save time (default value is `"0.2" "0.4" "0.6" "0.8"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_latency.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_latency.sh 0 >tmp_exp_latency.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get latency results of FarReach if any
$ awk -v flag=0 'flag == 0 && /\[exp2\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' tmp_exp_latency.out

# Get latency results of NetCache if any
$ awk -v flag=0 'flag == 0 && /\[exp2\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' tmp_exp_latency.out

# Get latency results of NoCache if any
$ awk -v flag=0 'flag == 0 && /\[exp2\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' tmp_exp_latency.out
```

### 4.3 Scalability Analysis

- **Pre-requisite: code is compiled with enabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_scalability.sh`:
	- You can keep a part of methods in `exp3_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of scalability values in `exp3_scalability_list` to save time (default value is `"32" "64" "128"`).
		- Note: you should also update `exp3_scalability_bottleneck_list` accordingly to keep a part of bottleneck indexes for your selected scalability values:
			- Bottleneck index 29 corresponding to scalability value 32.
			- Bottleneck index 59 corresponding to scalability value 64.
			- Bottleneck index 118 corresponding to scalability value 128.
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_scalability.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_scalability.sh 0 >tmp_exp_scalability.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any
$ TODO

# Get throughput results of NoCache if any
$ TODO

# Get throughput results of NetCache if any
$ TODO
```

## 5. AE on Synthetic Workloads

### 5.1 Impact of Write Ratio

- **Pre-requisite: code is compiled with enabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_write_ratio.sh`:
	- You can keep a part of methods in `exp4_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp4_workload_list` to save time (default value is `"synthetic-25" "synthetic-75" "synthetic"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_write_ratio.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_write_ratio.sh 0 >tmp_exp_write_ratio.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any
$ awk -v flag=0 'flag == 0 && /\[exp4\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_write_ratio.out

# Get throughput results of NoCache if any
$ awk -v flag=0 'flag == 0 && /\[exp4\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_write_ratio.out

# Get throughput results of NetCache if any
$ awk -v flag=0 'flag == 0 && /\[exp4\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_write_ratio.out
```

### 5.2 Impact of Key Distribution

- **Pre-requisite: code is compiled with enabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_key_distribution.sh`:
	- You can keep a part of methods in `exp5_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp5_workload_list` to save time (default value is `"skewness-90" "skewness-95" "uniform"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_key_distribution.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_key_distribution.sh 0 >tmp_exp_key_distribution.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any
$ TODO

# Get throughput results of NoCache if any
$ TODO

# Get throughput results of NetCache if any
$ TODO
```

### 5.3 Impact of Value Size

- **Pre-requisite: code is compiled with enabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_value_size.sh`:
	- You can keep a part of methods in `exp6_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp6_workload_list` to save time (default value is `"valuesize-16" "valuesize-32" "valuesize-64"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_value_size.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_value_size.sh 0 >tmp_exp_value_size.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any
$ TODO

# Get throughput results of NoCache if any
$ TODO

# Get throughput results of NetCache if any
$ TODO
```

### 5.4 Impact of Key Popularity Changes

- **Pre-requisite: code is compiled with disabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_dynamic.sh`:
	- You can keep a part of methods in `exp7_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp7_dynamic_rule_list` to save time (default value is `"hotin" "hotout" "random" "stable"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_dynamic.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_dynamic.sh 0 >tmp_exp_dynamic.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out

# Get throughput results of NoCache if any
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out

# Get throughput results of NetCache if any
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out
```

## 6. AE on Snapshot Generation and Crash Recovery

### 6.1 Performance of Snapshot Generation

- **Pre-requisite: code is compiled with disabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_snapshot.sh`:
	- You can keep a part of dynamic patterns in `exp8_dynamic_rule_list` to save time (default value is `"hotin" "hotout" "random"`).
	- You can keep a part of snapshot periods in `exp8_snapshot_list` to save time (default value is `"0" "2500" "5000" "7500" "10000"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_snapshot.sh <roundnumber>, where roundnumber is the index of current round
# You can run this script multiple times with different roundnumbers to get results of multiple rounds
$ nohup bash scripts/exps/run_exp_snapshot.sh 0 >tmp_exp_snapshot.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
	- If awk cannot find the results of average bandwidth cost, you may consider to use `bash scripts/local/calculate_bwcost_helper.h <statistics filepath>` to get results manually, where `<statistics filepath>` is a raw statistics file named `{EVALUATION_OUTPUT_PREFIX}/exp8/{roundnumber}/{snapshot period}_{dynamic patter}_tmp_controller_bwcost.out` if existing (e.g., `~/aeresults/exp8/0/2500_hotin_tmp_controller_bwcost.out`).
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get throughput and bandwidth results of hotin if any
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotin\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotin\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' tmp_exp_snapshot.out

# Get throughput results of hotout if any
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotout\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotout\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' tmp_exp_snapshot.out

# Get throughput results of random if any
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[random\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[random\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
```

### 6.2 Crash Recovery Time

- **Pre-requisite: code is compiled with enabling server rotation.**
- Under main client (dl11), for `scripts/exps/run_exp_recovery.sh`:
	- You can keep a part of round indexes in `exp9_round_list` to save time (default value is `"0" "1" "2" "3" "4" "5"`, i.e., 6 rounds).
	- You can keep a part of cache sizes in `exp9_cachesize_list` to save time (default value is `"100" "1000" "10000"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_recovery.sh <workloadmode = 0> <recoveryonly = 0>, where workloadmode = 0 means static pattern and recoveryonly = 0 means running a server rotation first to get raw statistics before crash recovery
# Note: this script does NOT support dynamic pattern now; if you really want to test crash recovery time of dynamic patterns (NOT in our paper), you can refer to the manual way for evaluation in our [README](./README.md)
# Note: we use recoveryonly = 0 by default; if you really want to use recoveryonly mode, see defails in [README](./README.md)
$ nohup bash scripts/exps/run_exp_recovery.sh 0 0 >tmp_exp_recovery.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get server-side and switch-side crash recovery time results of round index 0 if any
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[0\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' tmp_exp_recovery.out
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[0\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' tmp_exp_recovery.out

# Note: getting server-side and switch-side crash recovery time results of any other round index if any is similar as above
```
