from common import *

# Hot-in rules
for i in range(0, period_num): # 0, 2, ..., 5
    f = open(path_hotin+str(i), "w")
    for j in range(1, evict_num + 1): # 1, 2, ..., 200
        hotkey = j
        if i == 0:
            coldkey = j # NOTE: we do not change key popularity in the first second
        else:
            coldkey = max_key - i*evict_num + j
        f.write("{} {}\n".format(hotkey, coldkey)) # original hotkey to new coldkey
        f.write("{} {}\n".format(coldkey, hotkey)) # original coldkey to new hotkey
    f.flush()
    f.close()
