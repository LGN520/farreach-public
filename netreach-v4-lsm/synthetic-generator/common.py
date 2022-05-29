len_key = 16  #The number of bytes in the key
#len_val = 128 #The number of bytes in the value
max_key = 100000000  #The number of keys
max_hot = 10000 #The number of hot keys

path_kv = "synthetic-load.out"  #The path to save generated keys and values
path_hot = "synthetic-warmup.out" #The path to save the hot keys

path_query = "synthetic-run.out"
num_query = 32000000
zipf = 0.99
