# YCSB Raw Latency w/o Fixing Target Throughput

## Average latency

### Average Lantecy of round 1

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 311us | 383us | 443us | 702us  |  516us  |  378us |
| NetCache | 463us | 519us | 366us | 481us  |  337us  |  522us |
| FarReach | 551us | 447us | 266us | 373us  |  928us  |  427us |

### Average Lantecy of round 2

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 242us | 347us | 295us | 534us | 259us | 537us |
| NetCache | 251us | 260us | 820us | 666us | 475us | 971us |
| FarReach | 497us | 552us | 622us | 381us | 1168us | 421us |

### Average Lantecy of round 3

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 331us | 576us | 438us | --- | 447us | 377us |
| NetCache | 432us | 855us | 938us | 920us | 540us | 668us |
| FarReach | 791us | 1056us | 1148us | 964us | 1127us | 323us |

### Average Latency w/o Runtime Variation

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 294us | 435us | 392us | 618us | 407s | 431us |
| NetCache | 382us | 544us | 708us | 689us | 451us  |  720us  |
| FarReach | 613us | 685us | 678us | 573us  |  1074us  |  390us  |

## 95P latency

### 95P Lantecy of round 1

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 1655us | 1778us | 2480us | 2454us | 2690us | 1610us |
| NetCache | 2600us | 3021us | 1808us | 2012us | 2390us | 2175us | 
| FarReach | 2025us | 2188us | 1576us | 1698us | 2082us | 1501us | 

### 95P Lantecy of round 2

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 1814us | 1821us | 1794us | 2595us | 1705us | 2496us |
| NetCache | 2394us | 1902us | 2648us | 2309us | 2561us | 2227us |
| FarReach | 2012us | 2545us | 2440us | 1630us | 2538us | 1497us |

### 95P Lantecy of round 3

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 1755us | 2862us | 1721us | 1721us | 2372us | 1469us |
| NetCache | 2197us | 3287us | 2608us | 2219us | 2683us | 2262us |
| FarReach | 2181us | 2471us | 2628us | 1685us | 2251us | 1388us |

### 95P Latency w/o Runtime Variation

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 1741us | 2154us | 1998us | 2257us | 2256us | 1858us |
| NetCache | 2397us | 2737us | 2355us | 2180us | 2545us | 2221us | 
| FarReach | 2073us | 2401us | 2215us | 1671us | 2290us | 1462us | 

## 99P latency

### 99P Lantecy of round 1

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 2638us | 3028us | 2988us | 2701us | 2912us | 2597us |
| NetCache | 2994us | 3479us | 2562us | 2554us | 2731us | 2253us |
| FarReach | 3086us | 3497us | 2132us | 2521us | 2283us | 1668us |

### 99P Lantecy of round 2

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 1987us | 2868us | 2438us | 3091us | 1904us | 2684us |
| NetCache | 2861us | 2361us | 3531us | 2565us | 2897us | 2333us |
| FarReach | 2246us | 3685us | 3342us | 1867us | 3713us | 2658us |

### 99P Lantecy of round 3

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 2802us | 3389us | 3690us | 3690us | 2893us | 1649us |
| NetCache | 3170us | 3943us | 2801us | 2436us | 2953us | 2404us | 
| FarReach | 2424us | 3011us | 2796us | 1865us | 3016us | 1649us | 

### 99P Latency w/o Runtime Variation

| | YCSB A | YCSB B | YCSB C | YCSB D | YCSB F | LOAD |
| --- | --- | --- | --- | --- | --- | --- |
| NoCache | 2476us | 3095us | 3039us | 3161us | 2570us | 2310us |
| NetCache | 3008us | 3261us | 2965us | 2518us | 2860us | 2330us |
| FarReach | 2585us | 3398us | 2757us | 2084us | 3004us | 1992us |
