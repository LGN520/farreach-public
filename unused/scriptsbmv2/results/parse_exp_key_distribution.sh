set -x
#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: bash scriptsbmv2/reulsts/parse_exp_key_distribution.sh <filename>"
	exit
fi
tmp_outputfile=$1

echo "Get throughput results of FarReach if any"
awk -v flag=0 'flag == 0 && /\[exp5\]\[farreach\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get throughput results of NoCache if any"
awk -v flag=0 'flag == 0 && /\[exp5\]\[nocache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get throughput results of NetCache if any"
awk -v flag=0 'flag == 0 && /\[exp5\]\[netcache\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /aggregate throughput/ {flag = 0; print $0; next}' ${tmp_outputfile}
