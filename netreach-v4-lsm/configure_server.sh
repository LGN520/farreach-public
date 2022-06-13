sudo ifconfig enp129s0f0 10.0.1.13/24
sudo arp -s 10.0.1.11 3c:fd:fe:bb:ca:79

sudo sysctl -w net.core.rmem_max=8388608
sudo sysctl -w net.core.rmem_default=212992

ulimit -n 102400
