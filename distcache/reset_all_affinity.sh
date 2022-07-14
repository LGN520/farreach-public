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

sudo_passwd=123456

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

for cpu_coreidx in $(seq 0 $(expr ${server_cores} - 1))
do
	all_thread_infos=$(ps -eLF | awk -v a="$cpu_coreidx" '$9 == a {print;}')

	# Print affinity info of all server threads (you can check lwpid here)
	#echo "${all_thread_infos}"

	# NOTE: echo $param does not preserver "\n", while "$param" does
	all_thread_lwpids=( $(echo "${all_thread_infos}" | awk '{print $4}') )
	echo ${all_thread_lwpids[@]}

	# NOTE: ${#all_thread_lwpids} = ${#all_thread_lwpids[0]}, which counts # of characters in the first element of all_thread_lwpids
	# While ${#all_thread_lwpids[@]} = ${#all_thread_lwpids[*]}, which counts array length
	#echo ${#all_thread_lwpids}
	all_thread_num=${#all_thread_lwpids[@]}

	for tmpthread_idx in $(seq 0 $(expr ${all_thread_num} - 1))
	do
		tmpthread_lwpid=${all_thread_lwpids[${tmpthread_idx}]}

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
			echo "reset LWP ${tmpthread_lwpid} affinity from ${cpu_coreidx} to ${server_cores}-$(expr ${total_cores} - 1)"
			echo ${sudo_passwd} | sudo -S taskset -pc ${server_cores}-$(expr ${total_cores} - 1) ${tmpthread_lwpid}
		fi
	done
done

