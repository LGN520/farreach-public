# Experiment 1 - Old

## Settings

- Object size
	+ 16B key + 128B value
- Workloads
	+ YCSB A: 50% read + 50% write + 0.99 skewness
	+ YCSB B: 95% read + 5% write + 0.99 skewness
	+ YCSB C: 100% read + 0.99 skewness
	+ YCSB D: 95% read + 5% insert + latest (not skewed)
	+ YCSB E: 95% scan + 5% insert + uniform (not skewed)
	+ YCSB F: 50% read + 50% read-modify-write + 0.99 skewness
	+ Twitter trace 1: TODO
	+ Twitter trace 2: TODO
	+ Twitter trace 3: TODO
- Static pattern
	+ Simulate 16 servers by server rotation

## Throughput (MOPS)

|  Method   |    YCSB A     |     YCSB B     |       YCSB C       | YCSB D  | YCSB E  | YCSB F  | Trace 1  | Trace 2  | Trace 3  |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach  |  1.45/<u>1.89</u>  |  1.424/<u>1.58</u>  |    1.86/<u>1.89</u>     |  1.09   |         |  1.67   |
|  NoCache  |     0.76      |      0.85      |        0.91        |  1.14   |         |  1.01   |
| NetCache  |     0.82      |      1.05      | <u>1.58/1.69/1.521</u>  | <u>1.07</u>  |         | <u>1.04</u>  |

## Average latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 0.584/0.480 | 0.604/0.526 | 0.468/0.466 | 0.852 | | 0.532 |
| NetCache | 1.020 | 0.718 | 0.483 | 0.848 | | 0.915 |
| NoCache | 1.080 | 0.968 | 0.958 | 0.831 | | 0.940 |


## Medium latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 0.051/0.059 | 0.051/0.054 | 0.054/0.056 | 1.225 | | 0.052 | 
| NetCache | 0.710 | 0.104 | 0.053 | 1.081 | | 0.185 |
| NoCache | 1.626 | 1.416 | 1.410 | 1.128 | | 1.365 |

## 90P latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 2.114/1.598 | 2.131/1.874 | 1.628/1.524 | 1.645 | | 1.879 |
| NetCache | 2.071 | 2.204 | 1.541 | 1.652 | | 1.972 |
| NoCache | 1.822 | 1.602 | 1,577 | 1.594 | | 1.577 |

## 99P latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| FarReach | 2.324/1.900 | 2.421/2.135 | 1.892/1.832 | 1.814 | | 2.143 |
| NetCache | 2.238 | 2.573 | 2.335 | 1.960 | | 2.142 |
| NoCache | 2.027 | 1.757 | 1.712 | 1.788 | | 1.771 |
