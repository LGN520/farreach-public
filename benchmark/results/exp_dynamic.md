# Experiment 7 - Dynamic Workload

## Settings

- Object size
  + 16B key + 128B value
- Workloads
  + Synthetic: 100% write + 0.99 skewness
- Method
  + FarReach
  + NoCache
  + NetCache
- Evaluation pattern
  + Apply hotin/hotout/random dynamic rules with 2 servers

</br>

## Round 1 - Throughput (MOPS)
|  Method  | hotin | hotout | random | static |
|:--------:|:----------:|:----------:|:----------:|:----------:|
| FarReach | 0.223 | 0.233 | 0.234 | 0.205 |
| NoCache  | 0.137 | 0.135 | 0.142 | 0.133 |
| NetCache | 0.137 | 0.120 | 0.116 | 0.106 |
</br>

## Round 2 - Throughput (MOPS)
|  Method  | hotin | hotout | random | static |
|:--------:|:----------:|:----------:|:----------:|:----------:|
| FarReach | 0.221 | 0.228 | 0.208 | 0.230 |
| NoCache  | 0.115 | 0.131 | 0.133 | 0.136 |
| NetCache | 0.129 | 0.110 | 0.111 | 0.112 |
</br>

## Round 3 - Throughput (MOPS)
|  Method  | hotin | hotout | random | static |
|:--------:|:----------:|:----------:|:----------:|:----------:|
| FarReach | 0.219 | 0.226 | 0.174 | 0.210 |
| NoCache  | 0.122 | 0.123 | 0.130 | 0.136 |
| NetCache | 0.119 | 0.114 | 0.111 | 0.113 |
</br>

## Round 4 - Throughput (MOPS)
|  Method  | hotin | hotout | random | static |
|:--------:|:----------:|:----------:|:----------:|:----------:|
| FarReach | 0.207 | 0.207 | 0.215 | 0.218 |
| NoCache  | 0.123 | 0.123 | 0.119 | 0.129 |
| NetCache | 0.119 | 0.101 | 0.113 | 0.107 |
</br>

## Round 5 - Throughput (MOPS)
|  Method  | hotin | hotout | random | static |
|:--------:|:----------:|:----------:|:----------:|:----------:|
| FarReach | 0.222 | 0.231 | 0.221 | 0.228 |
| NoCache  | 0.127 | 0.131 | 0.136 | 0.137 |
| NetCache | 0.115 | 0.111 | 0.108 | 0.114 |
</br>


## Per-second Statistics:
- [round 1 statistics](./exp5-presecond/round1.txt)
- [round 2 statistics](./exp5-presecond/round2.txt)
- [round 3 statistics](./exp5-presecond/round3.txt)
- [round 4 statistics](./exp5-presecond/round4.txt)
- [round 4 statistics](./exp5-presecond/round5.txt)