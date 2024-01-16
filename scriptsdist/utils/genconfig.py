import re
import configparser

def genconfig(srcfile, dstfile, rules):
    # rules -> [(<key>,<value>)...]
    f = open(srcfile, "r")
    lines = f.readlines()
    f.close()
    f = open(dstfile, "w+")
    for line in lines:
        tmp_line = line
        for rule in rules:
            tmp_line = re.sub(rule[0], rule[1], tmp_line)
        f.writelines(tmp_line)
    f.close()


def gennewconfig(srcfile, dstfile, rules):
    config = configparser.ConfigParser()
    config.read(srcfile)
    for rule in rules:
        config[rule[0]][rule[1]] = rule[2]
    with open(dstfile, 'w') as configfile:
        config.write(configfile)
