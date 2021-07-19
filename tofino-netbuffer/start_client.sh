bash kill_client.sh
rm -rf tmp_client.out
rm -rf tmp_client*.out
./client </dev/null >tmp_client.out 2>&1 &
