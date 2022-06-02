from common import *

# Hot-in rules
for i in range(1, period_num + 1): # 1, 2, ..., 6
    f = open(path_hotin+str(i), "w")
    for j in range(1, evict_num + 1): # 1, 2, ..., 200
        hotkey = j
        coldkey = max_key - i*evict_num + j
        f.write("{} {}\n".format(hotkey, coldkey)) # original hotkey to new coldkey
        f.write("{} {}\n".format(coldkey, hotkey)) # original coldkey to new hotkey
    f.flush()
    f.close()
