import sys

if len(sys.argv) != 4:
    print "Usage: python sum_twofiles.py filea fileb filec"
    exit(-1)

filea_name = sys.argv[1]
fileb_name = sys.argv[2]
filec_name = sys.argv[3]

fda = open(filea_name, "r")
fdb = open(fileb_name, "r")
fdc = open(filec_name, "w+")

lineidx = 0
while True:
    linea = fda.readline()
    lineb = fdb.readline()

    if linea == "":
        break

    linea_elements = linea.split(" ")
    lineb_elements = lineb.split(" ")
    if len(linea_elements) != len(lineb_elements):
        print "Unmatched format of line {}".format(lineidx)
        exit(-1)

    linec = ""
    for i in range(len(linea_elements)):
        if (type(eval(linea_elements[i])) == float):
            tmp_element = float(linea_elements[i]) + float(lineb_elements[i])
        else:
            tmp_element = int(linea_elements[i]) + int(lineb_elements[i])
        if i == 0:
            linec = str(tmp_element)
        else:
            linec = "{} {}".format(linec, tmp_element)
    linec += "\n"
    fdc.write(linec)

    lineidx += 1

fda.close()
fdb.close()
fdc.flush()
fdc.close()
