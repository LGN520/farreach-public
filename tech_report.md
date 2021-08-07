# Tech Report

## Directory

- xindex
	+ Original xindex (single node mode)
- ovs
	+ Source code of OpenVSwitch
- [ovs-xindex-R](./ovs-xindex-R.md)
	+ Xindex with OVS-based transmission delay (linux kernel stack)
- [tofino-xindex-R](./tofino-xindex-R.md)
	+ Xindex with tofino-based transmission delay (linux kernel stack)
- [tofino-netbuffer](./tofino-netbuffer.md)
	+ Xindex with tofino-based netbuffer (linux kernel stack)
	+ Only support KV now (TODO: CBF, CDF model, backup KV)
- [tofino-xindex-dpdk](./tofino-xindex-dpdk.md)
	+ Xindex with tofino-based transmission delay (DPDK)
- [tofino-netbuffer-dpdk](./tofino-netbuffer-dpdk.md)
	+ Xindex with tofino-based netbuffer (DPDK)

## Global Configuration

- Configuration of XIndex
	+ See [README.md](./xindex/XIndex-R/READE.md)
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
	+ Install and configure DPDK
		* `cd dpdk-20.08/usertools`
		* `./dpdk-setup.sh`
			- Select option: 45 -> ERROR: Target does not have the DPDK UIO Kernel Module.
			- Solution: modify `dpdk-20.08/config/common_base`, set `CONFIG_RTE_EAL_IGB_UIO=y` (Line 107) and `CONFIG_RTE_LIBRTE_IEEE1588=y` (Line 156)
		* `modprobe uio`
			- NOTE: uio module is in /lib/modules/4.15.0-122-generic/kernel/drivers/uio
			- UIO API can register the user-space driver to map user-space buffer with the specified device
		* `./dpdk-setup.sh` for rebuilding
			- Select option: 38
			- Select option: 45
			- Select option: 51
			- Select PCI address: 0000:5e:00.1 (ens3f1)
				+ Not notifying since the interface is active
		* `./dpdk-devbind.py --b igb_uio 0000:5e:00.1`
			- Result still shows not notifying
			- Run the following commands
				+ `ifoncifg ens3f1 down`
				+ `sudo ./dpdk-devbind.py --b igb_uio 0000:5e:00.1`
				+ `./dpdk-devbind.py --status`
					* Now since OS cannot find ens3f1, ifconfig cannot configure this interface
					* To unbind the interface: `dpdk-devbind.py -u 0000:5e:00.1`, and then `reboot` (after reboot, need to reload UIO and IGB_UIO modules, and reset huge page num)
		* Configure huge page
			- `sudo sysctl -w vm.nr_hugepages=1024`
			- Run `cat /proc/meminfo | grep Huge` or `cat /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages` to check huge page size
		* Configure environment for DPDK
			- Add `export RTE_SDK=/home/ssy/dpdk/dpdk-20.08` into /etc/profile
			- Add `export RTE_TARGET=x86_64-native-linuxapp-gcc` into /etc/profile
	+ Test DPDK
		* `cd $RTE_SDK/examples/helloworld`
		* `make`
		* `sudo ./build/helloworld -l 0-1 -n 2`
		* UNSOLVED
			- How to run DPDK without root permission: we should use VA mode instead of PA mode for IOVA theoretically
			- `sudo ./app/test-pmd/build/app/testpmd -- -i --total-num-mbufs=2048` -> start -> stop -> non-zero RX/TX bytes: always zero without finding reasons
- Install TLDK (Deprecated)
	+ `git clone https://github.com/FDio/tldk.git`
	+ Install DPDK 18.11
	+ Change files for compatibility
		* `cd tldk`
		* `bash tldk_fix.sh` to solve error of `dereferencing pointer incomplete type`
	+ Test
		* `sudo ./x86_64-native-linuxapp-gcc/app/l4fwd -- --udp --mbuf-num 1000 --becfg ~/projects/NetBuffer/tofino-xindex-dpdk/client.cfg`
