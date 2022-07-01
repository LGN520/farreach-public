len_key = 16  #The number of bytes in the key
#len_val = 128 #The number of bytes in the value
max_key = 100000000  #The number of keys
max_hot = 10000 #The number of hot keys

path_kv = "synthetic-load.out"  #The path to save generated keys and values
path_hot = "synthetic-warmup.out" #The path to save the hot keys

path_query = "synthetic-run.out"
#num_query = 32000000
num_query = 100 * 1000 * 1000
zipf = 0.99

# six 10-sec periods in 60 secs
period_num = 7
# change popularity of 1000 keys in each period
evict_num = 1000
#evict_num = 10000
path_hotin = "synthetic-hotin.out"
