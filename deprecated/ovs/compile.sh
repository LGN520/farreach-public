make
make install
make modules_install

config_file="/etc/depmod.d/openvswitch.conf"

for module in datapath/linux/*.ko; do
	modname="$(basename ${module})"
	echo "override ${modname%.ko} * extra" >> $config_file
	echo "override ${modname%.ko} * weak-updates" >> $config_file
done

depmod -a
/sbin/modprobe openvswitch
/sbin/lsmod | grep openvswitch
