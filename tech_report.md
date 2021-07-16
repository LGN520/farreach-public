# Tech Report

## Part 1. OVS + XIndex (ovs-xindex-R)

### Configuration

- Install cmake
	+ `wget http://www.cmake.org/files/v3.5/cmake-3.5.1.tar.gz`
	+ `tar xf cmake-3.5.1.tar.gz`
	+ `cd cmake-3.5.1`
	+ `./configure`
	+ `make`
	+ `make install`
- Install OVS
	+ `git clone https://github.com/openvswitch/ovs.git`
	+ `./boot.sh`
	+ `./configure -with-linux=/lib/modules/$(uname -r)/build`
	+ `sudo ./compile.sh`
	+ `export PATH=$PATH:/usr/local/share/openvswitch/scripts`
	+ `ovs-ctl start`
	+ Check whether with successful install: `ovs-vsctl show`
	+ After modifying source code of OVS: `sudo ./remake.sh`
- Install DPDK
	+ Check environment
		* `lspci | grep Ethernet`
		* `enstool -i <if>`
		* Check [supported hardware](http://core.dpdk.org/supported/)
		* `wget https://fast.dpdk.org/rel/dpdk-20.08.tar.xz`
		* `xz -d dpdk-20.08.tar.xz`
		* `tar -xvf dpdk-20.08.tar`
		* `apt-get install numactl libnuma-dev`
		* `cd dpdk-20.08/usertools`
		* `./dpdk-setup.sh`
			- Select option: 45 -> ERROR: Target does not have the DPDK UIO Kernel Module.
			- (TODO) Solution: set `CONFIG_RTE_EAL_IGB_UIO=y` and `CONFIG_RTE_LIBRTE_IEEE1588=y` in dpdk-20.08/config

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

## Part 2. Tofino + XIndex (tofino-xindex-R)

### Configuration

The same as ovs-index.

### Implementation Log

- Add tofino-xindex-R

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

## Part 3. Tofino-based NetBuffer (tofino-netbuffer)

### Configuration

The same as above

### Implementation Log

- Add tofino-netbuffer
- Add update/register_update.py, controller.cpp, and test_controller.cpp
- Add tofino-netbuffer/controller
	+ Avoid the dependency on cmake 3.5 and gcc/g++ 7.0 (uncompatible in Tofino Debian OS)
- Modify tofino-netbuffer/tofino/basic.p4
	+ Due to outputing up to 32-bit metadata, we must store key/val_lo/hi independently
	+ Add KV component (key_lo/hi, value_lo/hi, and valid bit)
		* Support get
		* Support put (fix the bug with 3 days)
- Modify basic.p4
	+ Update IP length and UDP length when sending back packet from switch to client
- Modify client.cpp, server,cpp, raw_socket.cpp
	+ Use raw socket for sendto and recvfrom in client and server
- Update raw_socket.cpp
	+ Update IP checksum alg and UDP checksum alg
	+ Fix IP checksum error (ihl must be 4 for IPv4)
	+ Fix UDP length error (udp length must be in big endian)
- Update prepare.cpp
	+ To make sure the consistency of existing keys and non-existing keys
- Test correctness under one socket in server
	+ Get pass
		* KV hit pass
		* KV miss pass
	+ Put pass
		* Without eviction pass
		* With eviction pass
			* Modify original packet as put response and send it back
			* Clone a packet as put request for eviction (use PUT_REQ_S to notify servers that it does not require a response)
	+ TODO: Del pass
- TODO: add CBF for existence index
- TODO: create the same number of sockets in server to enable concurrency
- TODO: add backup KV for scan

### Run

- Prepare randomly-generated keys
	+ `cmake . -DCMAKE_BUILD_TYPE=Release`
	+ `make all`
	+ `./prepare`
	+ NOTE: We must keep the same exist_keys.out and nonexist_keys.out for client/server
- Run `cd tofino`
	+ Run `su` to enter root account
	+ Run `bash compile.sh` to compile p4 into binary code
	+ Run `bash start_switch.sh` to launch Tofino
	+ Create a new terminal and run `bash configure.sh` to configure data plane
- Run `bash start_server.sh` in server host
- Run `bash start_controller.sh` in Tofino OS
- Run `bash start_client.sh` in client host

## How to debug

- Use `ifconfig <if> promisc` to enable promisc mode
- Use `tcpdump -i <if> -e -vvv -X` to listen raw packets

## NOTES

- Ports usage
	+ Storage server: listen client requests on 1111
	+ NetBuffer controller: listen storage server notification on 2222
	+ Ptf: listen controller msg on 3333

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
- Tofino + NetBuffer
	+ Get error of "free invalid pointer" after running ptf -> bf_switchd_pd_lib_init: Assertion failed
		* Cannot upgrade g++/gcc to version 7. Rollback to g++/gcc version 4.9 is ok.
	+ Change egress_port as ingress_port to send the packet back: [Error] tofino increases TX packet, but the host cannot receive the packet (RX packet does not rise, and no error packet like CRC, frame, overrun, and dropped)
		* Swap MAC address: still fail
		* Enable PROMISC by `ifconfig <if> promisc`: still fail
		* Use raw socket: still fail
		* Use DPDK instead of kernel: TODO
		* SOLVED!
			- (1) Ethernet's src mac address cannot be the mac address of the receiver's interface (promisc mode only tolerates dst mac address)
			- (2) Tofino hardware error?: Cannot swap src/dst mac address -> no packet will be sent back with a large possibility
	+ Errno 22 of bind/sendto: wrong parameter
		* NOTE: sizeof(struct sockaddr) != sizeof(struct sockaddr_ll), you must give precise length
	+ Sth adds two extra bytes (both of zero) at the end of packet: unkown reason
	+ Invalid status of 128
		* Small/big endian is based on byte not bit
	+ If key 0 is evicted by key 1, get key 0 will not arrive storage server
		* Reason: if you do not assign value to condition_hi, it could be either 0 or 1
		* NOTE: predicate is 4-bit instead of 2-bit, which can only be 1/2/4/8
	+ No cloned packet and meta fields are zero
		* Must bind dev port and mirror id in control plane
		* All meta fields will be reset unless those in field list (packet fields are copied)
			- NOTE: clone function just marks a flag without any data dependency; it copies the packet fields at the stage when being invoked; it copies the field list at the end of ingress/egress
