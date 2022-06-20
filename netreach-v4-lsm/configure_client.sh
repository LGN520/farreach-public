sudo ifconfig enp129s0f1 10.0.1.11/24
sudo arp -s 10.0.1.13 3c:fd:fe:bb:c9:c8

sudo sysctl -w net.core.rmem_max=8388608
sudo sysctl -w net.core.rmem_default=212992

# Only work for current shell process (inherited by subprocess) -> so we need to source the script
ulimit -n 1024000
# Display
ulimit -n
