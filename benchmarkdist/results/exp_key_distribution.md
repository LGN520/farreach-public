# Experiment 5 - Impact of Skewness

## Settings

- Object size
  + 16B key + 128B value
- Workloads
  + Skewness-90: 100% write + 0.90 skewness
  + Skewness-95: 100% write + 0.95 skewness
  + Skewness-99: 100% write + 0.99 skewness
- Method
  + FarReach
  + NoCache
  + NetCache
- Evaluation pattern
	+ Simulate 16 servers by server rotation


## Round 1 - Throughput (MOPS)
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach | 1.438 | 1.536 | 1.997 | 1.012 |
| NoCache  | 1.047 | 0.927 | 0.855 | 1.047 |
| NetCache | 0.918 | 0.805 | 0.685 | 1.038 |
</br>

## Round 1 - Normalized throughput
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach |  26.069  |  22.126  |  26.076  |  15.971  |
| NoCache  |  14.019  |  12.686  |  11.041  |  15.760  |
| NetCache |  13.944  |  12.680  |  11.068  |  15.920  |
</br>

## Round 2 - Throughput (MOPS)
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach | 1.337 | 1.574 | 1.795 | 1.116 |
| NoCache  | 0.988 | 0.867 | 0.781 | 1.083 |
| NetCache | 0.895 | 0.784 | 0.646 | 1.084 |
</br>

## Round 2 - Normalized throughput
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach |  19.399  |  22.141  |  25.994  |  15.945  |
| NoCache  |  14.035  |  12.687  |  11.055  |  15.972  |
| NetCache |  14.217  |  12.558  |  11.024  |  15.986  |
</br>

## Round 3 - Throughput (MOPS)
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach | 1.377 | 1.651 | 1.847 | 1.129 |
| NoCache  | 1.053 | 0.935 | 0.848 | 1.088 |
| NetCache | 0.923 | 0.803 | 0.681 | 1.067 |
</br>

## Round 3 - Normalized throughput
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach |  19.423  |  22.158  |  26.095  |  15.799  |
| NoCache  |  14.231  |  12.675  |  11.009  |  15.929  |
| NetCache |  14.034  |  12.652  |  11.011  |  15.955  |
</br>

## Round 4 - Throughput (MOPS)
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach | 1.284 | 1.507 | 1.835 | 1.097 |
| NoCache  | 0.943 | 0.868 | 0.713 | 1.079 |
| NetCache | 0.848 | 0.742 | 0.631 | 1.070 |
</br>

## Round 4 - Normalized throughput
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach |  19.123  |  22.070  |  26.059  |  15.969  |
| NoCache  |  14.237  |  12.706  |  11.026  |  15.985  |
| NetCache |  14.205  |  12.492  |  11.051  |  15.942  |
</br>

## Round 5 - Throughput (MOPS)
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach | 1.272 | 1.565 | 1.850 | 1.087 |
| NoCache  | 0.968 | 0.838 | 0.766 | 1.034 |
| NetCache | 0.877 | 0.747 | 0.620 | 1.017 |
</br>

## Round 5 - Normalized throughput
|  Method  | 90 | 95 | 99 | Uniform |
|:--------:|:------:|:------:|:------:|:------:|
| FarReach |  19.151  |  22.078  |  26.047  |  15.983  |
| NoCache  |  14.049  |  12.489  |  11.050  |  15.938  |
| NetCache |  14.011  |  12.667  |  10.963  |  15.964  |
</br>
