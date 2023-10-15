import random
import math

from common import *

#Generate queries
f = open(path_query, "w")
for i in range(num_query):
    #Randomly select a key in uniform distribution
    key_header = random.randint(1, max_key)
    #key_body = [0] * (len_key - 4)
    
    #Save the generated query to the file
    f.write("UPDATE ")
    f.write(str(key_header) + '\n')
    #for i in range(len_key - 4):
    #    f.write(hex(key_body[i]) + ' ')
    #f.write('\n')
f.flush()
f.close()
