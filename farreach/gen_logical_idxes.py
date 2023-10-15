startidx = int(raw_input("Type startidx: "))
endidx = int(raw_input("Type endidx: "))

resultstr = ""
for i in range(startidx, endidx+1):
    if i == startidx:
        resultstr = "{}".format(i)
    else:
        resultstr = "{}:{}".format(resultstr, i)
print(resultstr)
