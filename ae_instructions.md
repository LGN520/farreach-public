# AE Instructions

- Here are the getting started instructions ([Section 4](#4-getting-started-instructions)) and detailed instructions starting from [Section 5](#5-ae-on-ycsb-core-workloads) to reproduce experiments in our paper.

- If you have any question, please contact us if available (sysheng21@cse.cuhk.edu.hk).

# Table of Contents

1. [Artifact Claims](#1-artifact-claims)
2. [AE Testbed Overview](#2-ae-testbed-overview)
3. [Notes for Artifact Evaluation](#3-notes-for-artifact-evaluation)
	1. [Preperation Already Done for AE](#31-preperation-already-done-for-ae)
	2. [Script Running Time](#32-script-running-time)
	3. [Possible Errors](#33-possible-errors)
4. [Getting Started Instructions](#4-getting-started-instructions)
	1. [Code Compilation to Disable Server Rotation](#41-code-compilation-to-disable-server-rotation)
	2. [Script Usage](#42-script-usage)
	3. [Code Compilation to Enable Server Rotation](#41-code-compilation-to-enable-server-rotation)
5. [AE on YCSB Core Workloads](#5-ae-on-ycsb-core-workloads)
	1. [Throughput Analysis](#51-throughput-analysis)
	2. [Latency Analysis](#52-latency-analysis)
	3. [Scalability Analysis](#53-scalability-analysis)
6. [AE on Synthetic Workloads](#6-ae-on-synthetic-workloads)
	1. [Impact of Write Ratio](#61-impact-of-write-ratio)
	2. [Impact of Key Distribution](#62-impact-of-key-distribution)
	3. [Impact of Value Size](#63-impact-of-value-size)
	4. [Impact of Key Popularity Changes](#64-impact-of-key-popularity-changes)
7. [AE on Snapshot Generation and Crash Recovery](#7-ae-on-snapshot-generation-and-crash-recovery)
	1. [Performance of Snapshot Generation](#71-performance-of-snapshot-generation)
	2. [Crash Recovery Time](#72-crash-recovery-time)
8. [AE on Hardware Resource Usage](#8-ae-on-hardware-resource-usage)

## 1. Artifact Claims

- First, we claim that **the results of experiments in AE may be different from those in our paper**, as the AE testbed is different from our evaluation testbed (unavailable now), including but not limited to different OSs, different numbers of CPU cores, different memory sizes, different disk I/O capabilities, different network interfaces, and different software packages.
	- For example, you may get larger throughput values of all methods due to more CPU cores (48) in AE testbed than those (24) in evaluation testbed.
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
- **Note that our AE testbed is managed by a springboard machine (dl1), to connect with any of the above machines:**
	+ Under your own machine (e.g., laptop): use the ssh command and private key provided in Artifact Description (submitted by hotcrp) to connect with the spingboard machine (dl1).
	+ Under the springboard machine (dl1): connect with any of the above machine by ssh (e.g., `ssh dl11`).

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

### 3.1 Preperation Already Done for AE

- **Note: we have finished system preparation and data preparation, such that you can directly execute getting started instructions and detailed instructions to reproduce experiments.**
	- System preparation includes software dependency installation, testbed configuration settings (e.g., Linux username, involved machines, and SSH settings), and code compilation.
	- Data preparation includes loading phase (pre-loading 100M records into server-side key-value storage) and workload analysis (calculate bottleneck partition by consistent hashing and generate workload files for server rotation).
	- Although we have configured `scripts/global.sh` based on AE testbed and the AE account (username: atc2023ae), **please also make a double-check on `scripts/global.sh` by yourself.**

</br>

- **Note: please ensure that our code is correctly compiled before running each experiment.**
	- **We have already compiled code to enable server rotation** for you, as most experiments need to enable server rotation.
	- If the experiment needs to disable server rotation but you have not disabled, please refer to [Section 4.1](#41-code-compilation-to-disable-server-rotation) for details.
	- If the experiment needs to enable server rotation but you have not enabled, please refer to [Section 4.3](#43-code-compilation-to-enable-server-rotation) for details.

### 3.2 Script Running Time

- **Note: most experiments use server rotation to simulate a large scale of servers, which is time-consuming:**
	- Each iteration fixes the bottleneck partition (and deploys a non-bottleneck partition) (**TIME: around 5 minutes**).
	- Each server rotation comprises tens of iterations (16-128) to simulate multiple machines (**TIME: around 1.5-12 hours**).
	- Each experiment needs multiple server rotations for different methods and parameter settings (**TIME: around 1-3 day(s)**).
	- Each round includes multiple experiments to evaluate from different perspectives (**TIME: around 1-2 week(s)**).
	- We need multiple rounds to reduce runtime variation (**TIME: around 1-2 month(s)**).
- **To save time, you can follow the guides of each experiment later to select a part of methods and parameter settings that you want before executing the corresponding script.**

### 3.3 Possible Errors

- **Note: if you get stuck at `[warmup_client] cache size: 10000` (from stdout of the current experiment script `scripts/exps/run_exp_*.sh`) with over ten minutes:**
	- Getting stuck can be caused by leftover processes of previous experiments, which are NOT killed successfully.
	- Under main client (dl11): run `bash scripts/remote/stopall.sh` to kill all involved processes.
	- Under each machine of clients and servers (dl11, dl20, dl21, and dl30):
		- Run `ps -aux | grep atc2023` to double-check if there exist any leftover process.
	- Under switch (bf3):
		- Run `ps -aux | grep atc2023` and `ps -aux | grep switch` to double-check if there exist any leftover process.

</br>

- **Note: if you miss a limited number of iterations of a server rotation:**
	- Missing iterations can be caused by different reasons (e.g., resource confliction and Tofino hardware bugs).
		- Experiment scripts will pose warning messages about which iterations are missed in which client.
	- You can simply re-run the entire experiment after killing all involved processes by `scripts/remote/stopall.sh`
	- Or you can only run each missed iteration for the experiment by `scripts/exps/run_makeup_rotation_exp.sh` (see how to perform a single iteration in [README.md](./README.md#32-perform-single-iteration)).

## 4. Getting Started Instructions

- **As our system does NOT have things like "Hello World", we take the detailed steps for the experiment of impact of key popularity changes, which has relatively short running time, as the getting started instructions.**
	- There are 12 numbers in total, each of which requires around 5-10 minutes.
	- You can refer the guides in [Script Usage](#42-script-usage) to reduce some methods and workloads to save time.

### 4.1 Code Compilation to Disable Server Rotation

- Under main client (dl11)
	- Enable server rotation:
		- Comment line 82 (//#define SERVER_ROTATION) in `common/helper.h` to enable server rotation.
		- Run `bash scripts/remote/sync_file.sh common helper.h` to sync the code change to all machines.
	- Re-compile code of FarReach:
		- Set `DIRNAME` as `farreach` in `scripts/common.sh`
		- Run `bash scripts/remote/sync_file.sh scripts common.sh`
		- Under main client (dl11) and secondary client (dl20), run `bash scripts/local/makeclient.sh`
		- Under first server (dl21) and second server (dl30), run `bash scripts/local/makeserver.sh`
		- Under Tofino switch (bf3), run `bash scripts/local/makeswitchos.sh`
	- Re-compile code of NoCache:
		- Set `DIRNAME` as `nocache` in `scripts/common.sh`
		- Run `bash scripts/remote/sync_file.sh scripts common.sh`
		- Under main client (dl11) and secondary client (dl20), run `bash scripts/local/makeclient.sh`
		- Under first server (dl21) and second server (dl30), run `bash scripts/local/makeserver.sh`
		- Under Tofino switch (bf3), run `bash scripts/local/makeswitchos.sh`
	- Re-compile code of NetCache:
		- Set `DIRNAME` as `netcache` in `scripts/common.sh`
		- Run `bash scripts/remote/sync_file.sh scripts common.sh`
		- Under main client (dl11) and secondary client (dl20), run `bash scripts/local/makeclient.sh`
		- Under first server (dl21) and second server (dl30), run `bash scripts/local/makeserver.sh`
		- Under Tofino switch (bf3), run `bash scripts/local/makeswitchos.sh`

### 4.2 Script Usage

- Under main client (dl11), for `scripts/exps/run_exp_dynamic.sh`:
	- You can keep a part of methods in `exp7_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp7_dynamic_rule_list` to save time (default value is `"hotin" "hotout" "random" "stable"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_dynamic.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_dynamic.sh 0 >tmp_exp_dynamic.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out

# Get throughput results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out

# Get throughput results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out
```

### 4.3 Code Compilation to Enable Server Rotation

- Under main client (dl11)
	- Enable server rotation:
		- Uncomment line 82 (#define SERVER_ROTATION) in `common/helper.h` to enable server rotation.
		- Run `bash scripts/remote/sync_file.sh common helper.h` to sync the code change to all machines.
	- Re-compile code of FarReach:
		- Set `DIRNAME` as `farreach` in `scripts/common.sh`
		- Run `bash scripts/remote/sync_file.sh scripts common.sh`
		- Under main client (dl11) and secondary client (dl20), run `bash scripts/local/makeclient.sh`
		- Under first server (dl21) and second server (dl30), run `bash scripts/local/makeserver.sh`
		- Under Tofino switch (bf3), run `bash scripts/local/makeswitchos.sh`
	- Re-compile code of NoCache:
		- Set `DIRNAME` as `nocache` in `scripts/common.sh`
		- Run `bash scripts/remote/sync_file.sh scripts common.sh`
		- Under main client (dl11) and secondary client (dl20), run `bash scripts/local/makeclient.sh`
		- Under first server (dl21) and second server (dl30), run `bash scripts/local/makeserver.sh`
		- Under Tofino switch (bf3), run `bash scripts/local/makeswitchos.sh`
	- Re-compile code of NetCache:
		- Set `DIRNAME` as `netcache` in `scripts/common.sh`
		- Run `bash scripts/remote/sync_file.sh scripts common.sh`
		- Under main client (dl11) and secondary client (dl20), run `bash scripts/local/makeclient.sh`
		- Under first server (dl21) and second server (dl30), run `bash scripts/local/makeserver.sh`
		- Under Tofino switch (bf3), run `bash scripts/local/makeswitchos.sh`

## 5. AE on YCSB Core Workloads

### 5.1 Throughput Analysis

- **Pre-requisite: code is compiled with enabling server rotation.**
	- If not, see [Section 4.3](#43-code-compilation-to-enable-server-rotation) to enable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_throughput.sh`:
	- You can keep a part of methods in `exp1_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp1_core_workload_list` to save time (default value is `"workloada" "workloadb" "workloadc" "workloadd" " workladf" "workload-load"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_throughput.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_throughput.sh 0 >tmp_exp_throughput.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp1\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_throughput.out

# Get throughput results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp1\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_throughput.out

# Get throughput results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp1\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_throughput.out
```

### 5.2 Latency Analysis

- **Pre-requisite: code is compiled with enabling server rotation.**
	- If not, see [Section 4.3](#43-code-compilation-to-enable-server-rotation) to enable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_latency.sh`:
	- You can keep a part of methods in `exp2_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of target throughput values `exp2_target_thpt_list` to save time (default value is `"0.2" "0.4" "0.6" "0.8"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_latency.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_latency.sh 0 >tmp_exp_latency.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get latency results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp2\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' tmp_exp_latency.out

# Get latency results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp2\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' tmp_exp_latency.out

# Get latency results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp2\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' tmp_exp_latency.out
```

### 5.3 Scalability Analysis

- **Pre-requisite: code is compiled with enabling server rotation.**
	- If not, see [Section 4.3](#43-code-compilation-to-enable-server-rotation) to enable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_scalability.sh`:
	- You can keep a part of methods in `exp3_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of scalability values in `exp3_scalability_list` to save time (default value is `"32" "64" "128"`).
		- Note: you should also update `exp3_scalability_bottleneck_list` accordingly to keep a part of bottleneck indexes for your selected scalability values:
			- Bottleneck index 29 corresponding to scalability value 32.
			- Bottleneck index 59 corresponding to scalability value 64.
			- Bottleneck index 118 corresponding to scalability value 128.
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_scalability.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_scalability.sh 0 >tmp_exp_scalability.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp3\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_scalability.out

# Get throughput results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp3\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_scalability.out

# Get throughput results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp3\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_scalability.out
```

## 6. AE on Synthetic Workloads

### 6.1 Impact of Write Ratio

- **Pre-requisite: code is compiled with enabling server rotation.**
	- If not, see [Section 4.3](#43-code-compilation-to-enable-server-rotation) to enable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_write_ratio.sh`:
	- You can keep a part of methods in `exp4_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp4_workload_list` to save time (default value is `"synthetic-25" "synthetic-75" "synthetic"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_write_ratio.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_write_ratio.sh 0 >tmp_exp_write_ratio.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp4\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_write_ratio.out

# Get throughput results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp4\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_write_ratio.out

# Get throughput results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp4\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_write_ratio.out
```

### 6.2 Impact of Key Distribution

- **Pre-requisite: code is compiled with enabling server rotation.**
	- If not, see [Section 4.3](#43-code-compilation-to-enable-server-rotation) to enable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_key_distribution.sh`:
	- You can keep a part of methods in `exp5_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp5_workload_list` to save time (default value is `"skewness-90" "skewness-95" "uniform"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_key_distribution.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_key_distribution.sh 0 >tmp_exp_key_distribution.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp5\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_key_distribution.out

# Get throughput results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp5\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_key_distribution.out

# Get throughput results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp5\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_key_distribution.out
```

### 6.3 Impact of Value Size

- **Pre-requisite: code is compiled with enabling server rotation.**
	- If not, see [Section 4.3](#43-code-compilation-to-enable-server-rotation) to enable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_value_size.sh`:
	- You can keep a part of methods in `exp6_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp6_workload_list` to save time (default value is `"valuesize-16" "valuesize-32" "valuesize-64"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_value_size.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_value_size.sh 0 >tmp_exp_value_size.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp6\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_value_size.out

# Get throughput results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp6\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_value_size.out

# Get throughput results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp6\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' tmp_exp_value_size.out
```

### 6.4 Impact of Key Popularity Changes

- **Pre-requisite: code is compiled with disabling server rotation.**
	- If not, see [Section 4.1](#41-code-compilation-to-disable-server-rotation) to disable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_dynamic.sh`:
	- You can keep a part of methods in `exp7_method_list` to save time (default value is `"farreach" "nocache" "netcache"`).
	- You can keep a part of workloads in `exp7_dynamic_rule_list` to save time (default value is `"hotin" "hotout" "random" "stable"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_dynamic.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_dynamic.sh 0 >tmp_exp_dynamic.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput results of FarReach if any.
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out

# Get throughput results of NoCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out

# Get throughput results of NetCache if any.
$ awk -v flag=0 'flag == 0 && /\[exp7\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_dynamic.out
```

## 7. AE on Snapshot Generation and Crash Recovery

### 7.1 Performance of Snapshot Generation

- **Pre-requisite: code is compiled with disabling server rotation.**
	- If not, see [Section 4.1](#41-code-compilation-to-disable-server-rotation) to disable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_snapshot.sh`:
	- You can keep a part of dynamic patterns in `exp8_dynamic_rule_list` to save time (default value is `"hotin" "hotout" "random"`).
	- You can keep a part of snapshot periods in `exp8_snapshot_list` to save time (default value is `"0" "2500" "5000" "7500" "10000"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**

```shell
# Usage: bash scripts/exps/run_exp_snapshot.sh <roundnumber>, where roundnumber is the index of current round.
# You can run this script multiple times with different roundnumbers to get results of multiple rounds.
$ nohup bash scripts/exps/run_exp_snapshot.sh 0 >tmp_exp_snapshot.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
	- If awk cannot find the results of average bandwidth cost, you may consider to use `bash scripts/local/calculate_bwcost_helper.h <statistics filepath>` to get results manually, where `<statistics filepath>` is a raw statistics file named `{EVALUATION_OUTPUT_PREFIX}/exp8/{roundnumber}/{snapshot period}_{dynamic patter}_tmp_controller_bwcost.out` if existing (e.g., `~/aeresults/exp8/0/2500_hotin_tmp_controller_bwcost.out`).
```shell
# Kill all involved processes.
$ bash scripts/remote/stopall.sh

# Get throughput and bandwidth results of hotin if any.
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotin\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotin\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' tmp_exp_snapshot.out

# Get throughput results of hotout if any.
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotout\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[hotout\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' tmp_exp_snapshot.out

# Get throughput results of random if any.
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[random\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
$ awk -v flag=0 'flag == 0 && /\[exp8\]\[random\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' tmp_exp_snapshot.out
```

### 7.2 Crash Recovery Time

- **Pre-requisite: code is compiled with enabling server rotation.**
	- If not, see [Section 4.3](#43-code-compilation-to-enable-server-rotation) to enable server rotation and re-compile code in all machines.
- Under main client (dl11), for `scripts/exps/run_exp_recovery.sh`:
	- You can keep a part of round indexes in `exp9_round_list` to save time (default value is `"0" "1" "2" "3" "4" "5"`, i.e., 6 rounds).
	- You can keep a part of cache sizes in `exp9_cachesize_list` to save time (default value is `"100" "1000" "10000"`).
	- **Note: do NOT launch any other experiment before this experiment finishes.**
<!-- 
	- Note: this script does NOT support dynamic pattern now **(NOT the experiments in our paper)**.
		- If you really want to test crash recovery time of dynamic patterns, you can refer to the manual way for evaluation in [README.md](./README.md#41-dynamic-worklaod-no-server-rotation), and get recovery time resutls by `scripts/remote/test_recovery_time.sh` and `scripts.local/calculate_recovery_time_helper.py`.
	- Note: we use recoveryonly = 0 by default.
		- If you really want to use recoveryonly mode (ONLY if you have alread run this experiment before), see the notes for exp_recovery in [README.md](./README.md#31-normal-script-usage).

```shell
# Usage: bash scripts/exps/run_exp_recovery.sh <workloadmode = 0> <recoveryonly = 0>.
# workloadmode = 0 means static pattern.
# recoveryonly = 0 means running a server rotation first to get raw statistics before crash recovery.
$ nohup bash scripts/exps/run_exp_recovery.sh 0 0 >tmp_exp_recovery.out 2>&1 &
```
-->

```shell
# Usage: bash scripts/exps/run_exp_recovery.sh
$ nohup bash scripts/exps/run_exp_recovery.sh >tmp_exp_recovery.out 2>&1 &
```

</br>

- After this experiment finishes, under main client (dl11):
```shell
# Kill all involved processes
$ bash scripts/remote/stopall.sh

# Get server-side and switch-side crash recovery time results of round index 0 if any.
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[0\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' tmp_exp_recovery.out
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[0\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' tmp_exp_recovery.out

# Note: getting server-side and switch-side crash recovery time results of any other round index is similar as above.
# But make sure that your `exp9_round_list` contains the rounde index when you execute the cript before.
# Take round index i as an example (you can replace {i} as a specific rounde index, e.g., 1):
# awk -v flag=0 'flag == 0 && /\[exp9\]\[{i}\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' tmp_exp_recovery.out
# awk -v flag=0 'flag == 0 && /\[exp9\]\[{i}\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' tmp_exp_recovery.out
```

## 8. AE on Hardware Resource Usage

- This experiment does NOT need to run the benchmark, as hardware resource usage is statically allocated after P4 code compilation, which is orthogonal with runtime environments.
	- **Note: we have already compiled P4 code of all methods (each takes around 3 hours), so you do NOT need to re-compile them again.**
- Under switch (bf3):
```shell
# Type su password to enter root mode.
$ su

# Enter the specific switch directory that stores compiled files of P4 code.
$ cd $SDE/pkgsrc/p4-build/tofino/

# Copy visualization files of FarReach, NoCache, and NetCache.
$ cp -r netbufferv4/visualization /home/atc2023ae/farreach_visualization
$ cp -r nocache/visualization /home/atc2023ae/nocache_visualization
$ cp -r netcache/visualization /home/atc2023ae/netcache_visualization

# Change ownership of copied files.
$ cd /home/atc2023ae
$ chown -R atc2023ae:atc2023ae /home/atc2023ae/farreach_visualization
$ chown -R atc2023ae:atc2023ae /home/atc2023ae/nocache_visualization
$ chown -R atc2023ae:atc2023ae /home/atc2023ae/netcache_visualization
```
- Under the springboard (dl1):
```
# Copy visualization files from switch to the springboard.
scp -r bf3:/home/atc2023ae/farreach_visualization /home/atc2023ae
scp -r bf3:/home/atc2023ae/nocache_visualization /home/atc2023ae
scp -r bf3:/home/atc2023ae/netcache_visualization /home/atc2023ae
```
- Under your own laptop (with any browser installed):
	- Use the scp command and private key provided in Artifact Description (submitted by hotcrp) to copy visualization files from springboard to your own laptop.
	- Check the visualization files (in html format) by any browser and count the hardware resource usage manually.
