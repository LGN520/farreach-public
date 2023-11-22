from __future__ import print_function

import sys

rowa = raw_input("Type row A: ")
rowb = raw_input("Type row B: ")
rowa = rowa.split()
rowb = rowb.split()

if len(rowa) != len(rowb):
    print("rowa should have the same length as row b!")
    exit(-1)

rowc = []
for i in range(len(rowa)):
    rowc.append(int(rowa[i]) + int(rowb[i]))

print("sum of two rows:", end=' ')
maxelement = -1
maxidx = -1
for i in range(len(rowc)):
    if i != len(rowc) - 1:
        print("{}".format(rowc[i]), end=' ')
    else:
        print("{}".format(rowc[i]), end='\n')
    if maxelement == -1 or maxelement < rowc[i]:
        maxelement = rowc[i]
        maxidx = i
print("max element: {}; max idx: {}".format(maxelement, maxidx))
