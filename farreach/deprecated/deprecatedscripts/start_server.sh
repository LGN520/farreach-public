set -x
bash localkill.sh server
rm -rf tmp_server.out
./server </dev/null >tmp_server.out 2>&1 &
