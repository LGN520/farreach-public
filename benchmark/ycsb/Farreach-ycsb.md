## README


### Compile

1. Compile certain database binding
   ```bash
   # farreach 
   mvn -pl site.ycsb:farreach-binding -am clean package

   # keydump
   mvn -pl site.ycsb:keydump-binding -am clean package
   ```
2. Compile for JNI libs
- Generate jni header files if you change java source code
    ```bash
	bash generate_jnifiles.sh core site.ycsb.SocketHelper
    ```
- Generate jni library files into jnilib/
    ```bash
	cd jnisrc; make all; cd ..
    ```
- NOTE: we should update jnisrc/site_ycsb_SocketHelper.\* accordingly after updating ../method/packet_format.\*
3. New parameters for running
    - `-pi` : physical index of current machine to run

<!-- - `-df` : path to the key dumping file -->

</br>

1. Dump workload statistics to ../output/workload-*.out
   ```bash
   ./bin/ycsb run keydump -pi <physical-index>
   ```
</br>

<!-- ./bin/ycsb run keydump -P workloads/workloada -pi <physical-index> -df <path-to-key-dumping-file> -p exportfile=<path-to-export-file> -->

2. Given a workload, generate dynamic rules to ../output.workload-*rules/
   ```bash
   python generate_dynamicrules.py <workloadname>
   ```
</br>

3Run farreach workload
   ```bash
   ./bin/ycsb run farreach -pi <physical-index> 
   ```
   
<!-- ./bin/ycsb run farreach -P workloads/workloada -threads <number-of-threads> -pi <physical-index> -p exportfile=<path-to-export-file> -->

### Design

1. Dependency relation
   
   Core -> InSwitchCache </br>
   Methods -> Core </br>

2. Farreach/Keydump compiling sequence
   
   ```bash
   YCSB Root
   InSwitch Cache Core
   Core YCSB
   YCSB Datastore Binding Parent
   FarReach DB Binding
   ```
   
3. Core Source Files
    - core/RemoteDB.java
    - core/workloads/PregenerateWorkload.java

3. Farreach Source Files
    - farreach/FarreachClient.java

4. Keydump Source Files
    - keydump/KeydumpClient.java
