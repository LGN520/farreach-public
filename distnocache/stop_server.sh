serverpid=$(ps -aux | grep "./server" | grep -v "grep" | awk '{print $2}')
kill -15 $serverpid
