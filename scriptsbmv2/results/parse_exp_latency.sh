set -x
#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: bash scriptsbmv2/reulsts/parse_exp_latency.sh <filename>"
	exit
fi
tmp_outputfile=$1

echo "Get latency results of FarReach if any"
awk -v flag=0 'flag == 0 && /\[exp2\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get latency results of NoCache if any"
awk -v flag=0 'flag == 0 && /\[exp2\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get latency results of NetCache if any"
awk -v flag=0 'flag == 0 && /\[exp2\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average latency/ {flag = 0; print $0; next}' ${tmp_outputfile}
