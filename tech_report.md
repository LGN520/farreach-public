# Tech Report

## OVS + XIndex (ovs-xindex-R)

### Configuration

- Install OVS
	+ `git clone https://github.com/openvswitch/ovs.git`
	+ `./boot.sh`
	+ `./configure -with-linux=/lib/modules/$(uname -r)/build`
	+ `sudo ./compile.sh`
	+ `export PATH=$PATH:/usr/local/share/openvswitch/scripts`
	+ `ovs-ctl start`
	+ Check whether with successful install: `ovs-vsctl show`
	+ After modifying source code of OVS: `sudo ./remake.sh`

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

## Tofino + XIndex (tofino-xindex-R)


### Configuration

The same as ovs-index.

### Run

- Prepare randomly-generated keys
	+ `cmake . -DCMAKE_BUILD_TYPE=Release`
	+ `make all`
	+ `./prepare`
- Run `cd tofino`
	+ Run `su` to enter root account
	+ Run `bash compile.sh` to compile p4 into binary code
	+ Run `bash start_switch.sh` to launch Tofino
	+ Create a new terminal and run `bash configure.sh` to configure data plane
- Run `bash start_server.sh` in server host
- Run `bash start_client.sh` in client host

## Fixed Issues

- XIndex
	+ Run `make microbench` -> Cannot support compile option `-faligned-new`
		* Update g++ from g++-5 to g++-7
			- `sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 10`
			- `sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 10`
			- `sudo add-apt-repository ppa:ubuntu-toolchain-r/test`
			- `sudo apt-get update`
			- `sudo apt-get install g++-7`
			- `sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 20`
			- `sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 20`
- OVS + XIndex
	+ NOTE: To support RCU mechanism, each frontend worker must send request one-by-one
	+ NOTE: For template programming, you must implement template class in header file to compile in one time, or explicitly tell the compiler which kinds of specified classes you will use for compiling individually as a library
	+ NOTE: SIGKILL cannot be catched or ignored by custom handler
- Test connectness of tofino by ping
	+ Must set correct IP and mask for each interface
	+ Must enable ports in tofino
	+ Must set correct ARP in hosts
	+ Must keep correct IP checksum, otherwise no ICMP reply
