import re

file = open('instrs.txt', 'r')
lines = file.readlines()
for line in lines:
    m = re.match(r".*\[\s*(\d+)\]:='(\w+)[\s|\S]*(true|false)[\s|\S]*(intsize|0);", line)
    if m is None:
        continue
    insq = m.group(4)
    if insq == "intsize":
        insq = "P5::int_size"
    print("ins_op(\"{}\", {}, {}, {})".format(m.group(2), m.group(1), m.group(3), insq))
