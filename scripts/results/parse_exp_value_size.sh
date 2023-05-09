#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: bash scripts/reulsts/parse_exp_recovery.sh <filename>"
	exit
fi
tmp_outputfile=$1

echo "Get server-side and switch-side crash recovery time of round 0 if any."
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[0\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' ${tmp_outputfile}
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[0\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get server-side and switch-side crash recovery time of round 1 if any."
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[1\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' ${tmp_outputfile}
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[1\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get server-side and switch-side crash recovery time of round 2 if any."
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[2\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' ${tmp_outputfile}
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[2\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get server-side and switch-side crash recovery time of round 3 if any."
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[3\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' ${tmp_outputfile}
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[3\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get server-side and switch-side crash recovery time of round 4 if any."
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[4\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' ${tmp_outputfile}
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[4\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' ${tmp_outputfile}

echo "Get server-side and switch-side crash recovery time of round 5 if any."
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[5\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Server total/ {flag = 0; print $0; next}' ${tmp_outputfile}
$ awk -v flag=0 'flag == 0 && /\[exp9\]\[5\]\[.*\] Get recovery time/ {flag = 1; print $0; next} flag == 1 && /Switch total/ {flag = 0; print $0; next}' ${tmp_outputfile}
