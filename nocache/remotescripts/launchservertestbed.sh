DIRNAME="nocache"

# NOTE: you need to launch switch data plane and local control plane before running this script

echo "clear tmp files in remote servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; rm tmp_server.out"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; rm tmp_server.out"

echo "launch servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >tmp_server.out 2>&1 &"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 1 >tmp_server.out 2>&1 &"
