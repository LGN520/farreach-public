import random
from common import *

random.seed(0)

f = open(path_kv, "w")
for i in range(1, max_key + 1):
    ## Generate a key-value item
    #Select a key
    key_header = i
    #key_body = [0] * (len_key - 4)
    #Select a value
    #val = [1] * len_val #The value
    ###################################################################################################
    
    ## Output the key and the value to the file
    f.write("UPDATE " + str(key_header) + "\n")
    #f.write(str(key_header) + " ")
    #for i in range(len(key_body)):
    #    f.write(hex(key_body[i]) + " ")
    #f.write("\n")
    
    #for i in range(len(val)):
    #    f.write(hex(val[i]) + " ")
    #f.write("\n\n")
    ###################################################################################################

f.flush()
f.close()
