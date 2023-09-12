import sys
import os
import mmap

if len(sys.argv) != 3:
    print("Usage: python3 trace-replay.py tracename phasenum")
    exit(-1)

tracename = sys.argv[1]
phasenum = int(sys.argv[2])
print("Tracename: {}; phasenum: {}".format(tracename, phasenum))
if not os.path.isfile(tracename):
    print("No such file: {}".format(tracename))
sortname = "sort.{}".format(tracename)
dumpname = "dump.{}".format(tracename)
if phasenum == 1 and not os.path.isfile(dumpname):
    print("No such file :{}".format(dumpname))

IBM_TRACE = "IBM"
TWEMCACHE_TRACE = "Twemcache"
#trace_type = IBM_TRACE 
trace_type = TWEMCACHE_TRACE 

if trace_type == IBM_TRACE:
    PUT_TYPE = "REST.PUT.OBJECT"
elif trace_type == TWEMCACHE_TRACE:
    PUT_TYPE = "set"
MAX_SWITCH_VALLEN = 128
# IBM trace001: 5; Twemcache cluster12: 19
HOTKEY_THRESHOLD = 19
# >=10MOPS (40Gbps): >=10K (1ms); <=100 (1us)
POPULATION_TIME = 10000
MAX_HOTKEY_CNT = 100

maxnum = -1
if phasenum == 0:
    key_writecnt_map = {}
    key_requestcnt_map = {}
    with open(tracename, "r") as f:
        mm = mmap.mmap(f.fileno(), 0, prot=mmap.PROT_READ)
        while True:
            if trace_type == IBM_TRACE:
                cur_elements = mm.readline().split()
                if len(cur_elements) == 0:
                    break;
                cur_type = cur_elements[1].decode('utf-8')
                cur_key = int(cur_elements[2].decode('utf-8'), 16) # key of IBM is integer
            elif trace_type == TWEMCACHE_TRACE:
                cur_elements = mm.readline().split(b",")
                if len(cur_elements) <= 1:
                    break;
                cur_key = cur_elements[1].decode('utf-8') # key if Twemcache is string
                cur_type = cur_elements[5].decode('utf-8')

            if cur_key not in key_requestcnt_map:
                key_requestcnt_map[cur_key] = 1
            else:
                key_requestcnt_map[cur_key] += 1
            if cur_type == PUT_TYPE:
                if cur_key not in key_writecnt_map:
                    key_writecnt_map[cur_key] = 1
                else:
                    key_writecnt_map[cur_key] += 1

    sorted_key_requestcnt_tuplelist = sorted(key_requestcnt_map.items(), key = lambda x: x[1], reverse=True)
    print("Sort key-requestcnt map into {}...".format(sortname))
    with open(sortname, "w") as f:
        for item in sorted_key_requestcnt_tuplelist:
            f.write("{} {}\n".format(item[0], item[1]))

    print("Dump key-writecnt map into {}...".format(dumpname))
    with open(dumpname, "w") as f:
        for k, v in key_writecnt_map.items():
            f.write("{} {}\n".format(k ,v))
elif phasenum == 1:
    key_writecnt_remain_map = {} # if the key remains write requests in the future
    with open(dumpname, "r") as f:
        mm = mmap.mmap(f.fileno(), 0, prot=mmap.PROT_READ)
        while True:
            cur_elements = mm.readline().split()
            if len(cur_elements) == 0:
                break;
            if trace_type == IBM_TRACE:
                cur_key = int(cur_elements[0].decode('utf-8'))
            else:
                cur_key = cur_elements[0].decode('utf-8')
            cur_writecnt = int(cur_elements[1].decode('utf-8'))
            key_writecnt_remain_map[cur_key] = cur_writecnt

    total_request_cnt = 0
    total_write_cnt = 0
    total_smallwrite_cnt = 0
    key_requestcnt_map = {}
    hotkey_set = set() # # of hot keys
    hotkey_withwrite_set = set() # # of hot keys with write requests
    hotkey_info_map = {} # <hotkey, [iscached, population_timer]>; # of hot keys with subsequent write requests
    subsequent_write_cnt = 0 # subsequent writes after the key being populated
    blocked_write_cnt = 0 # subsequent writes being blocked

    with open(tracename, "r") as f:
        mm = mmap.mmap(f.fileno(), 0, prot=mmap.PROT_READ)
        while True:
            if trace_type == IBM_TRACE:
                cur_elements = mm.readline().split()
                if len(cur_elements) == 0:
                    break;
                cur_type = cur_elements[1].decode('utf-8')
                cur_key = int(cur_elements[2].decode('utf-8'), 16) # key of IBM is integer
                if cur_type == PUT_TYPE: # DELETE does not have vallen
                    cur_vallen = int(cur_elements[3].decode('utf-8'))
            elif trace_type == TWEMCACHE_TRACE:
                cur_elements = mm.readline().split(b",")
                if len(cur_elements) <= 1:
                    break;
                cur_key = cur_elements[1].decode('utf-8') # key if Twemcache is string
                cur_vallen = int(cur_elements[3].decode('utf-8'))
                cur_type = cur_elements[5].decode('utf-8')

            total_request_cnt += 1

            if cur_key not in key_requestcnt_map:
                key_requestcnt_map[cur_key] = 1
            else:
                key_requestcnt_map[cur_key] += 1
            cur_requestcnt = key_requestcnt_map[cur_key]

            if cur_type == PUT_TYPE and cur_key in key_writecnt_remain_map:
                key_writecnt_remain_map[cur_key] -= 1

            # Being populated
            for hotkey, info in hotkey_info_map.items():
                if info[0] == False:
                    info[1] -= 1
                    if info[1] <= 0:
                        info[0] = True

            if cur_requestcnt >= HOTKEY_THRESHOLD:
                hotkey_set.add(cur_key) # current hot key
                if cur_key in key_writecnt_remain_map:
                    hotkey_withwrite_set.add(cur_key) # current hot key has write requests
                    # current hot key still has subsequent write requests in the future
                    if key_writecnt_remain_map[cur_key] > 0 and cur_key not in hotkey_info_map: 
                        hotkey_info_map[cur_key] = [False, POPULATION_TIME]
                        if (len(hotkey_info_map) >= MAX_HOTKEY_CNT):
                            print("Exceed MAX HOTKEY NUMBER {}!".format(MAX_HOTKEY_CNT))
                            exit()

            # Focus on write request
            if cur_type == PUT_TYPE:
                total_write_cnt += 1

                # NOTE: large write only invalidates in-switch cache; although write-through policy needs to block subsequent packets, write-back policy does not need blocking (large write is just a special write as delete)
                if cur_vallen <= MAX_SWITCH_VALLEN:
                    total_smallwrite_cnt += 1

                # NOTE: the write incurring cache population is served by server and not blocked
                # NOTE: No such subsequent write in IBM trace
                if cur_key in hotkey_info_map and hotkey_info_map[cur_key][1] != POPULATION_TIME:
                    subsequent_write_cnt += 1
                    if hotkey_info_map[cur_key][0] == False: # blocked if the key is still being populated
                        blocked_write_cnt += 1

            if maxnum != -1 and total_request_cnt >= maxnum:
                break
        mm.close()

    print("# of keys: {}; # of hot keys: {}; # of hot keys with write requests: {}; # of hot keys with subsequent write requests: {}".format(len(key_requestcnt_map), len(hotkey_set), len(hotkey_withwrite_set), len(hotkey_info_map)))
    print("# of total requests: {}; # of total writes: {}; # of total small writes: {}".format(total_request_cnt, total_write_cnt, total_smallwrite_cnt))
    print("Write ratio (# of writes / # of requests): {}".format(float(total_write_cnt)/float(total_request_cnt)))
    print("Small write ratio (# of small writes / # of writes): {}".format(float(total_smallwrite_cnt)/float(total_write_cnt)))
    if subsequent_write_cnt != 0:
        print("# of subsequent writes: {}; # of blocked writes: {}".format(subsequent_write_cnt, blocked_write_cnt))
        print("Blocked write ratio (# of blocked writes / # of subsequent writes): {}".format(float(blocked_write_cnt)/float(subsequent_write_cnt)))
