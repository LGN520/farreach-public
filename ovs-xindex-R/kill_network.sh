#!/usr/bin/env bash

ovs-vsctl del-port br0 veth1s
ip link set veth1s down
ip netns exec ns1 ifconfig veth1 0.0.0.0
ip netns exec ns1 ip link set veth1 down
ip netns exec ns1 ip link set veth1 netns 1
ip link delete veth1 type veth
#ip link delete veth1s type veth
ip netns del ns1

ovs-vsctl del-port br0 veth0s
ip link set veth0s down
ip netns exec ns0 ifconfig veth0 0.0.0.0
ip netns exec ns0 ip link set veth0 down
ip netns exec ns0 ip link set veth0 netns 1
ip link delete veth0 type veth
#ip link delete veth0s type veth
ip netns del ns0

ovs-vsctl del-br br0
