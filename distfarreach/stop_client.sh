clientpid=$(ps -aux | grep "./remote_client" | grep -v "grep" | awk '{print $2}')
kill -15 $clientpid
