# Experiment 1

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

|  Method   | YCSB A  | YCSB B  | YCSB C  | YCSB D | YCSB E  | YCSB F | Trace 1  | Trace 2  | Trace 3  |
|:---------:|:-------:|:-------:|:-------:|:------:|:-------:|:------:|:--------:|:--------:|:--------:|
| FarReach  |  1.744  |  1.544  |  1.876  | <u>1.09</u>  |    -    |  <u>1.67</u>  |
|  NoCache  |  0.853  |  0.922  |  0.911  | <u>1.14</u>  |    -    |  0.884   |
| NetCache  |  0.884  |  1.248  |  1.807  | <u>1.07</u>  |    -    |  <u>1.04</u>  |


## Average latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 0.489 | 0.561 | 0.461 | 0.852 | - | 0.532 |
| NoCache | 1.008 | 0.935 | 0.949 | 0.831 | - | 0.967 |
| NetCache | 0.956 | 0.690 | 0.475 | 0.848 | - | 0.915 |


## Medium latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 0.061 | 0.053 | 0.057 | 1.225 | - | 0.052 |
| NoCache | 1.491 | 1.369 | 1.417 | 1.128 | - | 1.429 |
| NetCache | 0.418 | 0.094 | 0.056 | 1.081 | - | 0.185 |


## 90P latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 1.562 | 1.999 | 1.597 | 1.645 | - | 1.879 |
| NoCache | 1.654 | 1.569 | 1,559 | 1.594 | - | 1.607 |
| NetCache | 2.011 | 1.817 | 1.526 | 1.652 | - | 1.972 |


## 95P latency (ms)
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 1.709 | 2.141 | 1.700 | 1.696 | - | 1.984 |
| NoCache | 1.695 | 1.612 | 1,594 | 1.652 | - | 1.653 |
| NetCache | 2.130 | 2.268 | 2.018 | 1.821 | - | 2.029 |


## 99P latency (ms)

| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB E | YCSB F | Trace 1 | Trace 2 | Trace 3 |
|:---------:|:-------------:|:--------------:|:------------------:|:-------:|:-------:|:-------:|:--------:|:--------:|:--------:|
| FarReach | 1.900 | 2.333 | 1.863 | 1.814 | - | 2.143 |
| NoCache | 1.875 | 1.718 | 1.689 | 1.788 | - | 1.776 |
| NetCache | 2.560 | 2.585 | 2.319 | 1.960 | - | 2.142 |
