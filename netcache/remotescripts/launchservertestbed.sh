DIRNAME="netcache"

# NOTE: you need to launch switch data plane and local control plane before running this script

echo "clear tmp files in remote servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; rm tmp_server.out; rm tmp_reflector.out; rm tmp_controller.out"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; rm tmp_server.out"

echo "launch controller"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./controller >tmp_controller.out 2>&1 &"

echo "launch servers"
ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 0 >tmp_server.out 2>&1 &"
ssh ssy@dl13 "cd projects/NetBuffer/${DIRNAME}; nohup ./server 1 >tmp_server.out 2>&1 &"

#echo "launch reflector"
#ssh ssy@dl16 "cd projects/NetBuffer/${DIRNAME}; nohup ./reflector >tmp_reflector.out 2>&1 &"
