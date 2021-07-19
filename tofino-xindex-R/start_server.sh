bash kill_server.sh
rm -rf tmp_server.out
./server </dev/null >tmp_server.out 2>&1 &
