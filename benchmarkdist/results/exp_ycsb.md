# Experiment 1 - Different Workloads

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
	+ Load: 100% load (not skewed)
	+ Twitter trace 1: cluster 9 (read-intensive + skewed) 
	+ Twitter trace 2: cluster 10 (write-intensive + uniform)
	+ Twitter trace 3: cluster 40 (write-intensive + skewed)
- Method
  + FarReach
  + NoCache
  + NetCache
- Evaluation pattern
	+ Simulate 16 servers by server rotation

</br>

## Workload Key Distribution
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| Hotest    |  0.39194487 |  0.39194487 |  0.39194487 |  0.00473398  |   0.58793422 |    |
| Near Hot  |  0.0286528  |  0.0286528  |  0.0286528  |  0.00286904  |   0.0429933  |    |
| Coldest   |  0.0001     |  0.0001     |  0.0001     |  0.0001      |   0.0001     |    |
</br>


## Round 1 Throughput (MOPS)
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load | Cluster 9 | Cluster 10 | Cluster 40 |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach  |  1.594  |  1.293  |  1.478  |  1.098  |   1.417  |  1.131  |  3.917  |  0.435  |  0.535  |
|  NoCache  |  0.765  |  0.796  |  0.732  |  1.108  |   0.829  |  1.135  |  1.542  |  0.500  |  0.593  |
| NetCache  |  0.819  |  1.057  |  1.580  |  1.132  |   0.899  |  1.080  |  3.845  |  0.478  |  0.544  |
</br>


## Round 1 Normalized throughput
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load | Trace 1 | Trace 2 | Trace 3 |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach |  26.076  |  25.865  |  25.986  |  15.957  |  24.269  |  15.786  |  58.849  |  16.000  |  15.542  |
| NoCache  |  11.028  |  11.021  |  11.037  |  15.675  |  11.030  |  15.794  |  13.157  |  15.545  |  15.527  |
| NetCache |  13.658  |  18.395  |  26.391  |  15.934  |  13.052  |  15.806  |  59.849  |  15.509  |  15.457  |
</br>


## Round 2 Throughput (MOPS)
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach  |  1.454  |  1.203  |  1.424  |  1.057  |  1.381  |  1.148  |
|  NoCache  |  0.783  |  0.751  |  0.753  |  0.999  |  0.763  |  1.117  |
| NetCache  |  0.831  |  0.984  |  1.351  |  1.020  |  0.823  |  1.104  |
</br>


## Round 2 Normalized throughput
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach |  26.353  |  25.835  |  25.953  |  15.926  |  25.696  |  16.000  |
| NoCache  |  11.019  |  11.019  |  11.018  |  15.889  |  11.052  |  15.800  |
| NetCache |  13.618  |  17.812  |  26.025  |  15.687  |  14.607  |  15.808  |
</br>

## Round 3 Throughput (MOPS)
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach  |  1.411  |  1.190  |  1.421  |  1.097  |  1.409  |  1.234  |
|  NoCache  |  0.758  |  0.786  |  0.819  |  1.001  |  0.837  |  1.185  |
| NetCache  |  0.767  |  1.049  |  1.476  |  1.108  |  0.901  |  1.081  |
</br>


## Round 3 Normalized throughput
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach |  25.962  |  25.716  |  25.990  |  15.898  |  25.787  |  15.985  |
| NoCache  |  11.037  |  11.006  |  11.022  |  15.653  |  11.033  |  16.000  |
| NetCache |  13.579  |  18.517  |  25.969  |  16.854  |  13.637  |  15.788  |
</br>

## Round 4 Throughput (MOPS)
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach  |  1.298  |  1.179  |  1.468  |  1.007  |  1.343  |  1.077  |
|  NoCache  |  0.690  |  0.787  |  0.804  |  1.051  |  0.791  |  1.023  |
| NetCache  |  0.677  |  1.005  |  1.446  |  0.993  |  0.831  |  1.027  |
</br>

## Round 4 Normalized throughput
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach |  25.944  |  26.357  |  25.978  |  15.973  |  26.360  |  15.768  |
| NoCache  |  11.055  |  11.036  |  11.050  |  15.920  |  11.005  |  15.799  |
| NetCache |  13.661  |  17.127  |  25.971  |  15.677  |  13.945  |  15.985  |
</br>

## Round 5 Throughput (MOPS)
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach  |  1.330  |  1.181  |  1.413  |  1.019  |  1.353  |  1.116  |
|  NoCache  |  0.740  |  0.791  |  0.803  |  1.050  |  0.800  |  1.115  |
| NetCache  |  0.783  |  0.958  |  1.486  |  1.033  |  0.894  |  1.090  |
</br>

## Round 5 Normalized throughput
| Method | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | Load |
|:--------:|:----------:|:----------:|:----------:|:----------:|:----------:|:----------:|
| FarReach |  25.809  |  26.357  |  25.994  |  15.718  |  26.270  |  16.014  |
| NoCache  |  11.027  |  11.042  |  11.050  |  15.696  |  11.050  |  15.764  |
| NetCache |  13.480  |  17.037  |  25.930  |  15.906  |  13.783  |  16.000  |
</br>
