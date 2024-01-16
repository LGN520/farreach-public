def genserverlist(maxservernumber,physical_server_scale):
    # res = ["0"]
    res =[]
    m = int(maxservernumber / physical_server_scale)
    # res.append(":".join(str(j) for j in range(1, m * 2)))
    for i in range(0, maxservernumber, m):
        res.append(":".join(str(j) for j in range(i, i + m)))
    return res

def find_server_index(input_list, target_number):
    for i, element in enumerate(input_list):
        if str(target_number) in element:
            return i
    return None

# print(genserverlist(128,8))