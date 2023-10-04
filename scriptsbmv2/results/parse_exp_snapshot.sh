#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/reulsts/parse_exp_snapshot.sh <filename>"
	exit
fi
tmp_outputfile=$1

echo "Get throughput and bandwidth results of hotin if any"
awk -v flag=0 'flag == 0 && /\[exp8\]\[hotin\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' ${tmp_outputfile}
awk -v flag=0 'flag == 0 && /\[exp8\]\[hotin\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get throughput and bandwidth results of hotout if any"
awk -v flag=0 'flag == 0 && /\[exp8\]\[hotout\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' ${tmp_outputfile}
awk -v flag=0 'flag == 0 && /\[exp8\]\[hotout\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get throughput and bandwidth results of random if any"
awk -v flag=0 'flag == 0 && /\[exp8\]\[random\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /avgthpt/ {flag = 0; print $0; next}' ${tmp_outputfile}
awk -v flag=0 'flag == 0 && /\[exp8\]\[random\]\[.*\] sync/ {flag = 1; print $0; next} flag == 1 && /average bwcost/ {flag = 0; print $0; next}' ${tmp_outputfile}
