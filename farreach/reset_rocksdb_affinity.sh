set -x
#!/usr/bin/env bash

if [ $# -lt 3 ]
then
	echo "Usage: ./reset_rocksdb_affinity.sh server_cores total_cores server_num worker_lwpids"
	echo "For example, ./reset_rocksdb_affinity.sh 32 48 2 6400 6401, all server threads except 6400 and 6401 bound with cpu cores [0, 31] will be reset into CPU cores [32, 47]"
	exit
fi

server_cores=$1
total_cores=$2
nonworker_corenum=$(expr ${total_cores} - ${server_cores})
server_num=$3
expected_paramnum=$(expr 3 + ${server_num})

if [ $# -ne ${expected_paramnum} ]
then
	echo "Usage: ./reset_rocksdb_affinity.sh server_cores total_cores server_num worker_lwpids"
	echo "For example, ./reset_rocksdb_affinity.sh 32 48 2 6400 6401, all server threads except 6400 and 6401 bound with cpu cores [0, 31] will be reset into CPU cores [32, 47]"
	exit
fi

# NOTE: $* = $@ = "$@" which treats parameters as multiple strings, whlie "$*" treats parameters as a single string
params=( $@ )
worker_lwpids=()
for i in $(seq 3 $(expr ${expected_paramnum} - 1))
do
	worker_lwpids=(${worker_lwpids[@]} ${params[${i}]})
done

server_thread_infos=$(ps -eLF | grep "./server" | grep -v "grep")

# Print affinity info of all server threads (you can check lwpid here)
echo "${server_thread_infos}"

# NOTE: echo $param does not preserver "\n", while "$param" does
server_thread_cores=( $(echo "${server_thread_infos}" | awk '{print $9}') )
server_thread_lwpids=( $(echo "${server_thread_infos}" | awk '{print $4}') )
echo ${server_thread_cores[@]}
echo ${server_thread_lwpids[@]}

# NOTE: ${#server_thread_cores} = ${#server_thread_cores[0]}, which counts # of characters in the first element of server_thread_cores
# While ${#server_thread_cores[@]} = ${#server_thread_cores[*]}, which counts array length
#echo ${#server_thread_cores}
server_thread_num=${#server_thread_cores[@]}

for tmpthread_idx in $(seq 0 $(expr ${server_thread_num} - 1))
do
	tmpthread_core=${server_thread_cores[${tmpthread_idx}]}
	tmpthread_lwpid=${server_thread_lwpids[${tmpthread_idx}]}

	# if tmpcore not in [0, server_cores - 1], we do not need to reset the affinity of the current thread
	if [ ${tmpthread_core} -ge ${server_cores} ]
	then
		continue
	fi

	isworker=0
	for workeridx in $(seq 0 $(expr ${#worker_lwpids[@]} - 1))
	do
		if [ ${worker_lwpids[${workeridx}]} -eq ${tmpthread_lwpid} ]
		then
			isworker=1
			break
		fi
	done

	if [ ${isworker} -eq 0 ]
	then
		#tmp_newcore=$(expr ${server_cores} + ${tmpthread_lwpid} % ${nonworker_corenum})
		#echo "reset LWP ${tmpthread_lwpid} affinity from ${tmpthread_core} to ${tmp_newcore}"
		echo "reset LWP ${tmpthread_lwpid} affinity from ${tmpthread_core} to ${server_cores}-$(expr ${total_cores} - 1)"
		taskset -pc ${server_cores}-$(expr ${total_cores} - 1) ${tmpthread_lwpid}
	fi
done

# Check results (NOTE: it may not show the latest CPU affinity)
#ps -eLF | grep "./server" | grep -v "grep"

