sudo ifconfig enp129s0f0 10.0.1.13/24
sudo arp -s 10.0.1.11 3c:fd:fe:bb:ca:79

sudo sysctl -w net.core.rmem_max=8388608
sudo sysctl -w net.core.rmem_default=212992

sudo hugeadm --thp-never

#sudo sysctl -w vm.dirty_background_ratio=5
#sudo sysctl -w vm.dirty_ratio=40

# Only work for current shell process (inherited by subprocess) -> so we need to source the script
ulimit -n 1024000
# Display
ulimit -n
