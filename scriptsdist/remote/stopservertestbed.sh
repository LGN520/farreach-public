echo "stop servers"
# bash scriptsdist/local/localstop.sh ./server >/dev/null 2>&1

echo "stop controller"
bash scriptsdist/local/localstop.sh ./controller >/dev/null 2>&1

sleep 15s # wait for database to finish flush and compaction

echo "kill servers"
bash scriptsdist/local/stopserver.sh >/dev/null 2>&1

echo "kill controller"
bash scriptsdist/local/localkill.sh ./controller >/dev/null 2>&1

