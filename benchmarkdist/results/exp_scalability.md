# Experiment 3 - Scalability

## Settings

- Object size
	+ 16B key + 128B value
- Workloads
	+ YCSB A: 50% read + 50% write + 0.99 skewness
- Method
  + FarReach
  + NoCache
  + NetCache
- Evaluation pattern
	+ Simulate 16/32/64/128 servers by server rotation

</br>

## Round 1 - Throughput (MOPS)
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 1.594 | 3.168 | 6.600 | 12.322 |
| NoCache  | 0.765 | 1.206 | 1.590 | 1.903 |
| NetCache | 0.819 | 1.168 | 1.551 | 1.957 |
</br>

## Round 1 - Normalized Throughput
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 26.076 | 51.950 | 103.605 | 205.331 |
| NoCache  | 11.028 | 15.021 | 18.817 | 21.964 |
| NetCache | 13.658 | 19.326 | 25.197 | 30.618 |
</br>

## Round 2 - Throughput (MOPS)
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 1.454 | 3.152 | 6.205 | 14.489 |
| NoCache  | 0.743 | 1.226 | 1.581 | 1.912 |
| NetCache | 0.766 | 1.228 | 1.608 | 1.932 |
</br>

## Round 2 - Normalized Throughput
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 26.353 | 51.881 | 103.446 | TODO |
| NoCache  | 11.019 | 14.976 | 18.806 | 21.999 |
| NetCache | 13.634 | 19.318 | 25.132 | 30.540 |
</br>

## Round 3 - Throughput (MOPS)
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 1.411 | 2.690 | 5.438 | 11.334 |
| NoCache  | 0.758 | 1.103 | 1.451 | 1.741 |
| NetCache | 0.767 | 1.146 | 1.501 | 1.777 |
</br>

## Round 3 - Normalized Throughput
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 25.962 | 51.742 | 102.887 | 205.576 |
| NoCache  | 11.037 | 14.973 | 18.775 | 21.943 |
| NetCache | 13.579 | 19.289 | 25.097 | 29.730 |
</br>

## Round 4 - Throughput (MOPS)
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 1.298 | 2.733 | 5.438 | 11.322 |
| NoCache  | 0.690 | 1.032 | 1.379 | 1.735 |
| NetCache | 0.677 | 1.080 | 1.451 | 1.785 |
</br>

## Round 4 - Normalized Throughput
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 25.944 | 51.594 | 102.887 | 205.382 |
| NoCache  | 11.055 | 14.990 | 18.808 | 21.990 |
| NetCache | 13.661 | 19.298 | 25.185 | 29.809 |
</br>


## Round 5 - Throughput (MOPS)
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 1.330 | 2.718 | 5.509 | 11.105 |
| NoCache  | 0.740 | 1.036 | 1.394 | 1.722 |
| NetCache | 0.783 | 1.062 | 1.467 | 1.795 |
</br>

## Round 5 - Normalized Throughput
|  Method  | 16 servers | 32 servers | 64 servers | 128 servers |
|:--------:|:----------:|:----------:|:----------:|:-----------:|
| FarReach | 25.809 | 51.726 | 103.381 | 203.669 |
| NoCache  | 11.027 | 14.956 | 18.753 | 21.861 |
| NetCache | 13.480 | 19.333 | 25.240 | 30.412 |
</br>