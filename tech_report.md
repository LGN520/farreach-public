# Tech Report

## OVS + XIndex

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

### Implementation

- Add prepare.cpp
	+ Save exist keys and nonexist keys (randomly generated) in disk for consistency between client and server
- Add packet_format.h, packet_format_impl,h, and client.cpp
	+ Send request from client by UDP packet

### Run

- NOTE: the arguments must be consistent for each executable module
- Prepare randomly-generated keys
	+ `cmake . -DCMAKE_BUILD_TYPE=Release`
	+ `make prepare`
	+ `./prepare`

## Fixed Issues

- XIndex
	+ Run `make microbench` -> Cannot support compile option `-faligned-new`
		* Update g++ from g++-5 to g++-7
- OVS + XIndex
	+ NOTE: To support RCU mechanism, each frontend worker must send request one-by-one
	+ NOTE: For template programming, you must implement template class in header file to compile in one time, or explicitly tell the compiler which kinds of specified classes you will use for compiling individually as a library
