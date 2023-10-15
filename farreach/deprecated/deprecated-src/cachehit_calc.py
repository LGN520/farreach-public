#!/usr/bin/env python

from numpy import random
import math

zipf = 0.99

num_query = 1000000
max_key = 1000000

print("Generate zipf distribution")

#Zipf
zeta = [0.0]
for i in range(1, max_key + 1):
    zeta.append(zeta[i - 1] + 1 / pow(i, zipf))
field = [0] * (num_query + 1)
k = 1
for i in range(1, num_query + 1):
    if (i > num_query * zeta[k] / zeta[max_key]):
        k = k + 1
    field[i] = k

print("Generate queries")

#Generate queries
frequency_map = {}
r_list = random.choice(range(1, num_query), num_query)
for i in range(num_query):
    #Randomly select a key in zipf distribution
    #r = random.randint(1, num_query)
    r = r_list[i]
    key_header = field[r]
    if key_header not in frequency_map:
        frequency_map[key_header] = 1
    else:
        frequency_map[key_header] += 1

print("Dump statistics")

hot_threshold = 10

hot_keynum = 0
hot_sumfrequency = 0
total_keynum = 0
total_sumfrequency = 0
for k, v in frequency_map.items():
    total_keynum += 1
    total_sumfrequency += v
    if v > hot_threshold:
        hot_keynum += 1
        hot_sumfrequency += v
print [hot_keynum, hot_sumfrequency, total_keynum, total_sumfrequency]
