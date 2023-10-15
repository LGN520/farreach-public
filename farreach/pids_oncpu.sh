set -x
#!/bin/bash

if [[ $# -ne 1 ]]; then
    echo 'One argument required for CPU number, e.g. "0" or "47"'
    exit 1
fi

ZeroCPU=$1

ps -eLF | head -n 1 # Get heading
ps -eLF | awk -v a="$ZeroCPU" '$9 == a {print;}'
