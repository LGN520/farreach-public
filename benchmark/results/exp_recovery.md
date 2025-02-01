# Experiment 9 - Crash Recovery Time

## Settings

- Object size
  + 16B key + 128B value
- Workloads
  + Synthetic: 100% write + 0.99 skewness
- Method
  + FarReach
- Static pattern
	+ Simulate 16 servers with server rotation with 100/1000/10000 in-switch cache size
- Statistics
	+ Server recovery time = server collect time + avg server preprocess time over 2 numbers + avg server replay time over 16 numbers
	+ Switch recovery time = switch collect time + switch replay


## Round 1 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.0 | 1.0 | 1.0 |
| Server Preprocess | 0.000324 | 0.001841 | 0.017104 |
| Server Replay     | 0.000228375 | 0.0009730625 | 0.0090649375 |
| **Server Recovery** | 1.000552375 | 1.0028140625 | 1.0261689375 |
| Switch Collect    | 1.0287 | 1.0068 | 1.0209 |
| Switch Replay     | 0.003955 | 0.035254 | 0.327371 |
| **Switch Recovery** | 1.032655 | 1.042054 | 1.348271 |
</br>

## Round 2 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.0 | 1.0 | 1.0 |
| Server Preprocess | 0.000337 | 0.001835 | 0.017125 |
| Server Replay     | 0.000259375 | 0.0010166875 | 0.0091796875 |
| **Server Recovery** | 1.000596375 | 1.0028516875 | 1.0263046875 |
| Switch Collect    | 1.0394 | 1.1927 | 1.0683 |
| Switch Replay     | 0.003732 | 0.035508 | 0.300064 |
| **Switch Recovery** | 1.043132 | 1.228208 | 1.368364 |
</br>

## Round 3 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.0 | 1.0 | 1.0 |
| Server Preprocess | 0.000325 | 0.001816 | 0.016991 |
| Server Replay     | 0.0002444375 | 0.0010104375 | 0.0106864375 |
| **Server Recovery** | 1.0005694375 | 1.0028264375 | 1.0276774375 |
| Switch Collect    | 0.9629 | 1.0652 | 0.9605 |
| Switch Replay     | 0.003833 | 0.035033 | 0.338202 |
| **Switch Recovery** | 0.966733 | 1.100233 | 1.298702 |
</br>

## Round 4 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.0 | 1.0 | 1.0 |
| Server Preprocess | 0.000333 | 0.001814 | 0.017021 |
| Server Replay     | 0.000252875 | 0.00102525 | 0.0086953125 |
| **Server Recovery** | 1.000585875 | 1.00283925 | 1.0257163125 |
| Switch Collect    | 0.9480 | 1.0497 | 1.0371 |
| Switch Replay     | 0.004487 | 0.030839 | 0.302612 |
| **Switch Recovery** | 0.952487 | 1.080539 | 1.339712 |
</br>

## Round 5 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.0 | 1.0 | 1.0 |
| Server Preprocess | 0.000332 | 0.001829 | 0.017086 |
| Server Replay     | 0.0002479375 | 0.0010289375 | 0.01002925 |
| **Server Recovery** | 1.0005799375 | 1.0028579375 | 1.02711525 |
| Switch Collect    | 1.0248 | 0.9861 | 1.0243 |
| Switch Replay     | 0.004986 | 0.032831 | 0.395596 |
| **Switch Recovery** | 1.029786 | 1.018931 | 1.419896 |
</br>


## Obselete Data
## Round 6 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.8673 | 1.8819 | 1.9606 |
| Server Preprocess | 0.012201 | 0.016416 | 0.108603 |
| Server Replay     | 0.002733 | 0.003579 | 0.012714 |
| **Server recovery** | 1.8822 | 1.9018 | 2.0819 |
| Switch Collect    | 1.4862 | 1.476 | 1.5622 |
| Switch Replay   | 0.014466 | 0.040288 | 0.335147 |
| **Switch recovery** | 1.5006 | 1.5163 | 1.8973 |
</br>

## Round 7 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.8989 | 1.8888 | 1.9473 |
| Server Preprocess | 0.012087 | 0.028939 | 0.29774 |
| Server Replay     | 0.014382 | 0.003412 | 0.01074 |
| **Server recovery** | 1.9254 | 1.9211 | 2.2557 |
| Switch Collect    | 1.4944 | 1.6885 | 1.5554 |
| Switch Recovery   | 0.014672 | 0.040997 | 0.294631 |
| **Switch recovery** | 1.509 | 1.7293 | 1.8499 |
</br>

## Round 8 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.8625 | 1.8797 | 1.9558 |
| Server Preprocess | 0.012242 | 0.048454 | 0.129369 |
| Server Replay     | 0.002714 | 0.003549 | 0.010820 |
| **Server recovery** | 1.8774 | 1.9316 | 2.0959 |
| Switch Collect    | 1.4868 | 1.4607 | 1.5591 |
| Switch Recovery   | 0.014699 | 0.040205 | 0.307982 |
| **Switch recovery** | 1.5013 | 1.5009 | 1.867 |
</br>

## Round 9 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.8612 | 1.8436 | 1.9504 |
| Server Preprocess | 0.012265 | 0.016411 | 0.076786 |
| Server Replay     | 0.002630 | 0.003800 | 0.010903 |
| **Server recovery** | 1.876 | 1.86381 | 2.03808 |
| Switch Collect    | 1.468 | 1.4618 | 1.5518 |
| Switch Recovery   | 0.013857 | 0.043106 | 0.304768 |
| **Switch recovery** | 1.4818 | 1.5049 | 1.8565 |
</br>

## Round 10 - Recovery Time
|  Method  | 100 | 1000 | 10000 |
|:--------:|:----------:|:----------:|:----------:|
| Server Collect    | 1.853 | 2.006 | 1.973 |
| Server Preprocess | 0.012127 | 0.016669 | 0.0971240 |
| Server Replay     | 0.003158 | 0.003989 | 0.0138137 |
| **Server recovery** | 1.8683 | 2.026 | 2.0838 |
| Switch Collect    | 1.4699 | 1.4622 | 1.5504 |
| Switch Recovery   | 0.014414 | 0.042576 | 0.296493 |
| **Switch recovery** | 1.4843 | 1.5047 | 1.8468 |
</br>
