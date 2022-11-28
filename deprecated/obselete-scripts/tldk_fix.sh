# NOTE: -i means inplace edit, -r means recursive, -l means output the names of files with content matching the pattern
sed -i "s/rte_tcp_hdr/tcp_hdr/g" `grep rte_tcp_hdr -rl ./`
sed -i "s/rte_udp_hdr/udp_hdr/g" `grep rte_udp_hdr -rl ./`
sed -i "s/rte_ipv4_hdr/ipv4_hdr/g" `grep rte_ipv4_hdr -rl ./`
sed -i "s/rte_ipv6_hdr/ipv6_hdr/g" `grep rte_ipv6_hdr -rl ./`
sed -i "s/rte_ether_hdr/ether_hdr/g" `grep rte_ether_hdr -rl ./`
sed -i "s/rte_vlan_hdr/vlan_hdr/g" `grep rte_vlan_hdr -rl ./`
sed -i "s/rte_arp_hdr/arp_hdr/g" `grep rte_arp_hdr -rl ./`
sed -i "s/rte_ether_addr/ether_addr/g" `grep "ether_addr" -rl ./`
sed -i "s/RTE_ETHER_MTU/ETHER_MTU/g" `grep RTE_ETHER_MTU -rl ./`
sed -i "s/RTE_ETHER_CRC_LEN/ETHER_CRC_LEN/g" `grep RTE_ETHER_CRC_LEN -rl ./`
sed -i "s/RTE_ETHER_MAX_JUMBO_FRAME_LEN/ETHER_MAX_JUMBO_FRAME_LEN/g" `grep RTE_ETHER_MAX_JUMBO_FRAME_LEN -rl ./`
sed -i "s/RTE_ETHER_MAX_LEN/ETHER_MAX_LEN/g" `grep RTE_ETHER_MAX_LEN -rl ./`
sed -i "s/RTE_ETHER_TYPE_IPV6/ETHER_TYPE_IPv6/g" `grep RTE_ETHER_TYPE_IPV6 -rl ./`
sed -i "s/RTE_ETHER_TYPE_IPV4/ETHER_TYPE_IPv4/g" `grep RTE_ETHER_TYPE_IPV4 -rl ./`
sed -i "s/RTE_ETHER_TYPE_VLAN/ETHER_TYPE_VLAN/g" `grep RTE_ETHER_TYPE_VLAN -rl ./`
sed -i "s/RTE_ETHER_TYPE_ARP/ETHER_TYPE_ARP/g" `grep RTE_ETHER_TYPE_ARP -rl ./`
sed -i "s/RTE_ARP_OP_REQUEST/ARP_OP_REQUEST/g" `grep RTE_ARP_OP_REQUEST -rl ./`
sed -i "s/arp_protocol/arp_pro/g" `grep arp_protocol -rl ./`
sed -i "s/arp_hardware/arp_hrd/g" `grep arp_hardware -rl ./`
sed -i "s/arp_opcode/arp_op/g" `grep arp_opcode -rl ./`
sed -i "s/RTE_ARP_HRD_ETHER/ARP_HRD_ETHER/g" `grep RTE_ARP_HRD_ETHER -rl ./`
sed -i "s/RTE_IPV4_HDR_DF_FLAG/IPV4_HDR_DF_FLAG/g" `grep RTE_IPV4_HDR_DF_FLAG -rl ./`
sed -i "s/RTE_IPV4_HDR_IHL_MASK/IPV4_HDR_IHL_MASK/g" `grep RTE_IPV4_HDR_IHL_MASK -rl ./`
sed -i "s/RTE_IPV4_IHL_MULTIPLIER/IPV4_IHL_MULTIPLIER/g" `grep RTE_IPV4_IHL_MULTIPLIER -rl ./`
sed -i "s/rte_arp_ipv4/arp_ipv4/g" `grep rte_arp_ipv4 -rl ./`
sed -i "s/RTE_ARP_OP_REPLY/ARP_OP_REPLY/g" `grep RTE_ARP_OP_REPLY -rl ./`

