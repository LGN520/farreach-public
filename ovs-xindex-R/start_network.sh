#!/usr/bin/env bash

ovs-vsctl add-br br0

ip netns add ns0
ip link add veth0 type veth peer name veth0s
ip link set veth0 netns ns0
ip netns exec ns0 ip link set veth0 up
ip netns exec ns0 ifconfig veth0 10.0.0.1/24
ip link set veth0s up
ovs-vsctl add-port br0 veth0s

ip netns add ns1
ip link add veth1 type veth peer name veth1s
ip link set veth1 netns ns1
ip netns exec ns1 ip link set veth1 up
ip netns exec ns1 ifconfig veth1 10.0.0.2/24
ip link set veth1s up
ovs-vsctl add-port br0 veth1s
