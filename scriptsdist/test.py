#!/bin/python
import json
import numpy as np

with open(
    "/root/farreach-private/benchmarkdist/output/latest-statistics/farreach-hotin-client1.out"
) as f:
    python_dict = json.load(f)

# for key in python_dict:
#     print(key.keys())

print(len(python_dict))
print(python_dict[0].keys())
# perserverCachehits
# perserverOpsdone
# totalOpsdone
sumCachehits = np.zeros(128)
sumperserverCachehits= np.zeros(128)
sumtotalOpsdone = 0
serverOpsdone = np.zeros(128)

for item in python_dict:
    sumperserverCachehits+=np.array(item["perserverCachehits"])
    tmpcachesum=np.sum(np.array(item["perserverCachehits"]))
    # print(tmpcachesum)
    # print(item["perserverOpsdone"])
    # print(item["totalOpsdone"])
    sumCachehits += np.array(item["perserverCachehits"])
    sumtotalOpsdone += item["totalOpsdone"]
    serverOpsdone = serverOpsdone + np.array(item["perserverOpsdone"])
print(serverOpsdone)


sum_of_squares = np.sum(np.square(serverOpsdone))
sum_of_elements = np.sum(serverOpsdone)
# Jain's fairness index
fairness_index = (sum_of_elements ** 2) / (len(serverOpsdone) * sum_of_squares)
print('cache hit ratio',np.sum(sumCachehits) / sumtotalOpsdone)
print(f"Jain's fairness index: {fairness_index}")
# Jain's fairness index: 0.9357433921326883
with open(
    "/root/farreach-private/benchmarkdist/output/latest-statistics/farreach-hotin-client0.out"
) as f:
    python_dict = json.load(f)


for item in python_dict:
    sumperserverCachehits+=np.array(item["perserverCachehits"])
    tmpcachesum=np.sum(np.array(item["perserverCachehits"]))
    # print(tmpcachesum)
    # print(item["perserverOpsdone"])
    # print(item["totalOpsdone"])
    sumCachehits += np.array(item["perserverCachehits"])
    sumtotalOpsdone += item["totalOpsdone"]
    serverOpsdone = serverOpsdone + np.array(item["perserverOpsdone"])

print(serverOpsdone)
print(sumperserverCachehits)

sum_of_squares = np.sum(np.square(serverOpsdone))
sum_of_elements = np.sum(serverOpsdone)
# Jain's fairness index
fairness_index = (sum_of_elements ** 2) / (len(serverOpsdone) * sum_of_squares)
print('cache hit ratio',np.sum(sumCachehits) / sumtotalOpsdone)
print(f"Jain's fairness index: {fairness_index}")


print(np.argmax(serverOpsdone),serverOpsdone[np.argmax(serverOpsdone)])
print(np.argmax(sumperserverCachehits),sumperserverCachehits[np.argmax(sumperserverCachehits)])