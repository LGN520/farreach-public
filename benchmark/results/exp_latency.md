# Experiment 2 - Overall Analysis

## Settings

- Object size
	+ 16B key + 128B value
- Workloads
	+ YCSB A: 50% read + 50% write + 0.99 skewness
- Method
  + FarReach
  + NoCache
  + NetCache
- Evaluation pattern:
  + Throughput & imbalanced ratio: simulate 16 servers by server rotation
  + Latency: simulate 16 servers by server rotation with 0.2/0.4/0.6/0.8 aggregated target throughput

</br>	


## Round 1 Throughput (MOPS)
|  Method  |  Throughput  |
|:--------:|:----------:|
| FarReach | 1.594 |
| NoCache  | 0.765 |
| NetCache | 0.819 |
</br>

## Round 1 - Imbalanced Ratio
|  Method  |  Imbalanced Ratio  |
|:--------:|:----------:|
| FarReach | 1.027 |
| NoCache  | 1.786 |
| NetCache | 1.657 |
</br>

## Round 1 - Average Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.070 | 0.084/0.133/0.101 | 0.087 | 0.082 |
| NoCache  | 0.077 | 0.102/0.103/0.094 | 0.097 | 0.276 |
| NetCache | 0.073 | 0.080 | 0.114 | 0.571 |
</br>

## Round 1 - Medium Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.056 | 0.057/0.057/0.058 | 0.058 | 0.060 |
| NoCache  | 0.060 | 0.064/0.062/0.062 | 0.069 | 0.079 |
| NetCache | 0.061 | 0.065 | 0.072 | 0.087 |
</br>

## Round 1 - 90P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.079 | 0.088/0.094/0.090 | 0.096 | 0.112 |
| NoCache  | 0.087 | 0.112/0.101/0.097 | 0.117 | 1.187 |
| NetCache | 0.086 | 0.103 | 0.144 | 2.676 |
</br>

## Round 1 - 95P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.096 | 0.113/0.144/0.118 | 0.123 | 0.148 |
| NoCache  | 0.107 | 0.155/0.133/0.126 | 0.150 | 1.644 |
| NetCache | 0.103 | 0.127 | 0.199 | 2.811 |
</br>


----

## Round 2 Throughput (MOPS)
|  Method  |  Throughput  |
|:--------:|:----------:|
| FarReach | 1.495 |
| NoCache  | 0.783 |
| NetCache | 0.831 |
</br>

## Round 2 - Imbalanced Ratio
|  Method  |  Imbalanced Ratio  |
|:--------:|:----------:|
| FarReach | 1.018 |
| NoCache  | 1.789 |
| NetCache | 1.641 |
</br>

## Round 2 - Average Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.083 | 0.109 | 0.098 | 0.120 |
| NoCache  | 0.089 | 0.139 | 0.157 | 0.424 |
| NetCache | 0.091 | 0.127 | 0.166 | 0.351 |
</br>

## Round 2 - Medium Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.057 | 0.057 | 0.058 | 0.060 |
| NoCache  | 0.059 | 0.064 | 0.071 | 0.090 |
| NetCache | 0.061 | 0.064 | 0.070 | 0.086 |
</br>

## Round 2 - 90P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.092 | 0.096 | 0.103 | 0.126 |
| NoCache  | 0.089 | 0.116 | 0.219 | 1.491 |
| NetCache | 0.090 | 0.109 | 0.143 | 1.261 |
</br>

## Round 2 - 95P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.119 | 0.136 | 0.136 | 0.247 |
| NoCache  | 0.112 | 0.186 | 0.725 | 2.848 |
| NetCache | 0.112 | 0.145 | 0.278 | 2.126 |
</br>

----

## Round 3 Throughput (MOPS)
|  Method  |  Throughput  |
|:--------:|:----------:|
| FarReach | 1.411 |
| NoCache  | 0.758 |
| NetCache | 0.767 |
</br>

## Round 3 - Imbalanced Ratio
|  Method  |  Imbalanced Ratio  |
|:--------:|:----------:|
| FarReach | 1.018 |
| NoCache  | 1.750 |
| NetCache | 1.667 |
</br>

## Round 3 - Average Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.091 | 0.110 | 0.112 | 0.110 |
| NoCache  | 0.089 | 0.094 | 0.326 | 0.314 |
| NetCache | 0.088 | 0.117 | 0.154 | 0.440 |
</br>

## Round 3 - Medium Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.057 | 0.058 | 0.060 | 0.062 |
| NoCache  | 0.061 | 0.064 | 0.072 | 0.081 |
| NetCache | 0.063 | 0.065 | 0.075 | 0.091 |
</br>

## Round 3 - 90P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.089 | 0.097 | 0.112 | 0.134 |
| NoCache  | 0.091 | 0.102 | 0.151 | 1.468 |
| NetCache | 0.094 | 0.110 | 0.183 | 1.969 |
</br>

## Round 3 - 95P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.128 | 0.147 | 0.275 | 0.256 |
| NoCache  | 0.113 | 0.131 | 0.334 | 1.751 |
| NetCache | 0.115 | 0.144 | 0.386 | 2.496 |
</br>

----

## Round 4 Throughput (MOPS)
|  Method  |  Throughput  |
|:--------:|:----------:|
| FarReach | 1.298 |
| NoCache  | 0.690 |
| NetCache | 0.677 |
</br>

## Round 4 - Imbalanced Ratio
|  Method  |  Imbalanced Ratio  |
|:--------:|:----------:|
| FarReach | 1.019 |
| NoCache  | 1.765 |
| NetCache | 1.655 |
</br>

## Round 4 - Average Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.102 | 0.092 | 0.125 | 0.141 |
| NoCache  | 0.096 | 0.099 | 0.155 | 0.321 |
| NetCache | 0.091 | 0.100 | 0.228 | 0.298 |
</br>

## Round 4 - Medium Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.057 | 0.058 | 0.060 | 0.061 |
| NoCache  | 0.061 | 0.063 | 0.072 | 0.079 |
| NetCache | 0.061 | 0.065 | 0.074 | 0.076 |
</br>

## Round 4 - 90P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.087 | 0.095 | 0.126 | 0.150 |
| NoCache  | 0.093 | 0.103 | 0.151 | 1.597 |
| NetCache | 0.092 | 0.108 | 0.212 | 1.391 |
</br>

## Round 4 - 95P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.117 | 0.127 | 0.215 | 0.436 |
| NoCache  | 0.120 | 0.137 | 0.334 | 2.973 |
| NetCache | 0.119 | 0.139 | 1.359 | 2.008 |
</br>


----

## Round 5 Throughput (MOPS)
|  Method  |  Throughput  |
|:--------:|:----------:|
| FarReach | 1.330 |
| NoCache  | 0.740 |
| NetCache | 0.783 |
</br>

## Round 5 - Imbalanced Ratio
|  Method  |  Imbalanced Ratio  |
|:--------:|:----------:|
| FarReach | 1.023 |
| NoCache  | 1.771 |
| NetCache | 1.673 |
</br>

## Round 5 - Average Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.095 | 0.096 | 0.116 | 0.143 |
| NoCache  | 0.101 | 0.105 | 0.222 | 0.365 |
| NetCache | 0.092 | 0.107 | 0.123 | 0.472 |
</br>

## Round 5 - Medium Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.057 | 0.058 | 0.060 | 0.062 |
| NoCache  | 0.059 | 0.064 | 0.074 | 0.074 |
| NetCache | 0.062 | 0.065 | 0.069 | 0.077 |
</br>

## Round 5 - 90P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.090 | 0.098 | 0.115 | 0.150 |
| NoCache  | 0.091 | 0.107 | 0.200 | 1.816 |
| NetCache | 0.093 | 0.111 | 0.135 | 2.246 |
</br>

## Round 5 - 95P Latency (ms)
|  Method  | 0.2 | 0.4 | 0.6 | 0.8 |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 0.123 | 0.142 | 0.174 | 0.339 |
| NoCache  | 0.123 | 0.142 | 1.664 | 1.998 |
| NetCache | 0.115 | 0.143 | 0.190 | 3.210 |
</br>

