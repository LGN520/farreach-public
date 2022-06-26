DIRNAME="netreach-v4-lsm"

# NOTE: you need to update config,ini (especially server1.server_logical_idxes) before running the script

# TODO: retrieve dl15.bottleneckserver to the state just after loading phase
echo "(1) stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash stop_server.sh"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash stop_server.sh"
echo "(2) stop clients"
bash stop_client.sh
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; bash stop_client.sh"

sleep 1s

echo "(3) sync config.ini"
bash sync_file.sh config.ini

echo "(4) start servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >tmp.out 2>&1 &"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 1 >tmp.out 2>&1 &"

sleep 5s

echo "(5) start clients"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; nohup ./remote_client 1 >tmp.out 2>&1 &"
sleep 1s
./remote_client 0

echo "(6) stop servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; bash stop_server.sh"
ssh ssy@dl15 "cd projects/NetBuffer/${DIRNAME}; bash stop_server.sh"
