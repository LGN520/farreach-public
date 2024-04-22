# def genserverlist(maxservernumber,physical_server_scale):
#     # res = ["0"]
#     res =[]
#     m = int(maxservernumber / physical_server_scale)
#     # res.append(":".join(str(j) for j in range(1, m * 2)))
#     for i in range(0, maxservernumber, m):
#         if maxservernumber - (i + m) < m:
#             res.append(":".join(str(j) for j in range(i, maxservernumber)))
#         else : 
#             res.append(":".join(str(j) for j in range(i, i + m)))

#     print(res)
#     return res
def genserverlist(m, n):

    per_part = m // n
    remainder = m % n

    result = []
    start = 0
    for i in range(n):
        if i < remainder:
            end = start + per_part + 1
        else:
            end = start + per_part
        result.append(':'.join(map(str, range(start, end))))
        start = end

    return result
def find_server_index(input_list, target_number):
    for i, element in enumerate(input_list):
        if str(target_number) in element:
            return i
    return None

# print(generate_list(16,2))