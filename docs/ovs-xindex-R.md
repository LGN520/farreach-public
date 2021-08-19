## OVS + XIndex (ovs-xindex-R)

### Implementation Log

- Add prepare.cpp
	+ Save exist keys and nonexist keys (randomly generated) in disk for consistency between client and server
- Add packet_format.h and packet_format_impl.h
	+ Implement packet_type_t, packet_t, and get_request_t
- Add client.cpp
	+ Client sends request as UDP packet to server and receives corresponding response
- Add server.cpp
	+ Server receives request from client -> invoke XIndex APIs -> sends response to client
- Update packet_format.h, packet_format_impl.h
	+ Implement get_response_t
- Add kill_server.sh
	+ Send SIGTERM to kill server
- Add start_client.sh, kill_client.sh, and start_server.sh to launch client and server in netns
- Add start_network.sh and kill_network.sh
	+ Construct the virtual network topology: client (veth0) -> (veth0s) OVS (veth1s) -> (veth1) server
- Update packet_format.h, packet_format_impl.h, client.cpp, and server.cpp
	+ Support all APIs including get, put, del, and scan

### Run

- NOTE: the arguments must be consistent for each executable module
- Prepare randomly-generated keys
	+ `cmake . -DCMAKE_BUILD_TYPE=Release`
	+ `make all`
	+ `./prepare`
- Construct virtual network
	+ `sudo bash start_network.sh`
- Launch server in background
	+ `sudo ip netns exec ns1 bash start_server.sh`
- Launch client in background
	+ `sudo ip netns exec ns0 bash start_client.sh`
- Clean up the mess
	+ `sudo ip netns exec ns0 bash kill_server.sh`
	+ `sudo ip netns exec ns0 bash kill_client.sh`
	+ `sudo ip netns exec ns0 bash kill_network.sh`
