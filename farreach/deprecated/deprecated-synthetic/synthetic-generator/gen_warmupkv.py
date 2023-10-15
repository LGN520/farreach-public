import random
from common import *

random.seed(0)

f_hot = open(path_hot, "w")
#f.write(str(max_key) + "\n\n")
for i in range(1, max_hot + 1):
    ## Generate a key-value item
    #Select a key
    key_header = i
    #key_body = [0] * (len_key - 4)
    #Select a value
    #val = [1] * len_val #The value
    ###################################################################################################

    ##Output the hot key to the file
    f_hot.write("UPDATE " + str(key_header) + "\n")
    #    f_hot.write(str(key_header) + " ")
    #    for i in range(len(key_body)):
    #        f_hot.write(hex(key_body[i]) + " ")
    #    f_hot.write("\n")
    ###################################################################################################

f_hot.flush()
f_hot.close()
