# Tech Report

## Switch configuration

- There is sth wrong with dl31:ens3f0 (tofino:1/0)
- We use dl31:ens3f1(tofino:2/0) and dl32:ens3f1(tofino:3/0)

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
	+ See [README.md](../xindex/XIndex-R/READE.md)
	+ For jemalloc, follow the next instructions to install libjemalloc-dev instead of libjemalloc1 as XIndex
- Install jemalloc
	+ `wget https://github.com/jemalloc/jemalloc/archive/refs/tags/5.2.1.tar.gz`
	+ `gunzip 5.2.1.tar.gz`
	+ `tar -xvf 5.2.1.tar`
	+ `cd jemalloc-5.2.1`
	+ `./autogen.sh`
	+ `make dist`
	+ `make`
	+ `sudo make install`
		* Installed into /usr/local/bin, /usr/local/lib, and /usr/local/include
- Install boost
	+ `sudo apt-get install libboost-all-dev`
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
		* Build and bind
			* IGB UIO
				* `./dpdk-setup.sh`
					- Select option: 45 -> ERROR: Target does not have the DPDK UIO Kernel Module.
					- Solution: modify `dpdk-20.08/config/common_base`, set `CONFIG_RTE_EAL_IGB_UIO=y` (Line 107) and `CONFIG_RTE_LIBRTE_IEEE1588=y` (Line 156)
				* `modprobe uio`
					- NOTE: uio module is in /lib/modules/4.15.0-122-generic/kernel/drivers/uio
					- UIO API can register the user-space driver to map user-space buffer with the specified device
				* `./dpdk-setup.sh` for rebuilding
					- Select option: 38 to build DPDK
					- Select option: 45 to insert IGB_UIO
					- Select option: 51 to bind port
					- Select PCI address: 0000:5e:00.1 (ens3f1)
						+ Not notifying since the interface is active
				* `./dpdk-devbind.py --b igb_uio 0000:5e:00.1`
					- Result still shows not notifying
					- Run the following commands
						+ `ifconfig ens3f1 down`
						+ `sudo ./dpdk-devbind.py --b igb_uio 0000:5e:00.1`
						+ `./dpdk-devbind.py --status`
							* Now since OS cannot find ens3f1, ifconfig cannot configure this interface
			* VFIO
				* Check IOMMU: `uname -r`; `dmesg | grep -e DMAR -e IOMMU`; `cat /proc/cmdline | grep iommu=pt`; `cat /proc/cmdline | grep intel_iommu=on`;
					- If IOMMU is not enabled: set `GRUB_CMDLINE_LINUX_DEFAULT="quiet splash iommu=pt intel_iommu=on"` in /etc/default/grub; `sudo update-grub`; `sudo reboot`
				* `./dpdk-setup.sh`
					- Select option: 38 to build DPDK
					- Select option: 46 to insert VFIO
					- `ifconfig ens3f1 down`
					- ./dpdk-devbind.py --b vfio-pci 0000:5e:00.1
			* To unbind the interface, we can use dpdk-setup.sh to change the driver from vfio to i40e
				* Deprecated: To unbind the interface: `dpdk-devbind.py -u 0000:5e:00.1`, and then `reboot` (after reboot, need to reload UIO and IGB_UIO modules, and reset huge page num)
		* Configure huge page
			- `sudo sysctl -w vm.nr_hugepages=1024`
			- Run `cat /proc/meminfo | grep Huge` or `cat /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages` to check huge page size
		* Configure environment for DPDK
			- Add `export RTE_SDK=/home/ssy/dpdk/dpdk-20.08` into /etc/profile
			- Add `export RTE_TARGET=x86_64-native-linuxapp-gcc` into /etc/profile
	+ Test DPDK
		* Helloworld
			* `cd $RTE_SDK/examples/helloworld`
			* `make`
			* `sudo ./build/helloworld -l 0-1 -n 2`
				- NOTE: `EAL: No available hugepages reported in hugepages-1048576kB` is normal since my huge page size is 2KB
		* Skeleton
			* `cd $RTE_SDK/examples/skeleton`
			* `make`
			* `sudo ./build/basicfwd -l 0-3 -n 4`
				- Error: no available port
				- Solution
					+ (1) Change DPDL from 20.08 to 18.11 to match the kernel driver version and firmware version of the PCI device
					+ (3) Change driver from igb_uio to vfio
					+ (4) Use Makefile (refer to [tas](https://github.com/tcp-acceleration-service/tas/blob/master/Makefile))
					+ NOT USE: Change `CONFIG_RTE_LIBRTE_BNX2X_PMD=n` to `CONFIG_RTE_LIBRTE_BNX2X_PMD=y` in $RTE_SDK/config/common_base
		* Notes
			- How to run DPDK without root permission?
				+ We should use VA mode instead of PA mode for IOVA theoretically
			- `sudo ./app/test-pmd/build/app/testpmd -- -i --total-num-mbufs=2048` -> start -> stop -> non-zero TX/RX packets
				+ If always zero, (1) the port is not connected to NIC or switch; for tofino, the port is not correctly enabled; (2) You must have odd ports
- Install TLDK (Deprecated)
	+ `git clone https://github.com/FDio/tldk.git`
	+ Install DPDK 18.11
	+ Change files for compatibility
		* `cd tldk`
		* `bash tldk_fix.sh` to solve error of `dereferencing pointer incomplete type`
	+ Test
		* `sudo ./x86_64-native-linuxapp-gcc/app/l4fwd -- --udp --mbuf-num 1000 --becfg ~/projects/NetBuffer/tofino-xindex-dpdk/client.cfg`
- Install RocksDB
	+ `sudo apt-get install libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev`
	+ `wget https://github.com/facebook/rocksdb/archive/refs/tags/v6.22.1.tar.gz`
	+ `gunzip v6.22.1.tar.gz`
	+ `tar -xvf v6.22.1.tar`
	+ `cd rocksdb-6.22.1`
	+ `make static_lib`
	+ Issues
		* Error: Compression type Snappy is not linked with the binary
			- Install libsnappy and remake static lib of rocksdb
			- Prepare the directory for database in advance
