#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/reulsts/parse_exp_dynamic.sh <filename>"
	exit
fi
tmp_outputfile=$1

echo "Get throughput results of FarReach if any"
awk -v flag=0 'flag == 0 && /\[exp7\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get throughput results of NoCache if any"
awk -v flag=0 'flag == 0 && /\[exp7\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get throughput results of NetCache if any"
awk -v flag=0 'flag == 0 && /\[exp7\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' ${tmp_outputfile}
