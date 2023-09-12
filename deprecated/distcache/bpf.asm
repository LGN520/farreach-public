# use ./bpf_asm -c bpf.asm to generate sock filter for raw socket in spine reflector

# filter for ipv4
ldh [12]
jne #0x0800, drop
# filter for udp
ldb [23]
jne #0x11, drop
# filter for udp.dstport as dp2cpserver port of spine reflector
ldh [36]
jne #5018, drop
ret #-1
drop: ret #0
