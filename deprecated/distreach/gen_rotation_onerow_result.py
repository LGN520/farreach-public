# NOTE: this script helps to generate each result line w/ one/two server threads
# Input line 0: bottleneck_pktcnt [rotate_pktcnt] total_pktcnt avg_thpt avg_latency at client 0
# Input line 1: bottleneck_pktcnt [rotate_pktcnt] total_pktcnt avg_thpt avg_latency at client 1

line0 = raw_input("Type result of client 0: ")
line1 = raw_input("Type result of clinet 1: ")
line0_elements = line0.split(" ")
line1_elements = line1.split(" ")

result = []
for i in range(len(line0_elements)):
    result.append(float(line0_elements[i]) + float(line1_elements[i]))

if len(result) == 4:
    print("result: {} {} {} {}".format(\
            int(result[0]), \
            int(result[1]), \
            float(result[2]), \
            float(result[3]/2.0)))
elif len(result) == 5:
    print("result: {} {} {} {} {}".format(\
            int(result[0]), \
            int(result[1]), \
            int(result[2]), \
            float(result[3]), \
            float(result[4]/2.0)))
