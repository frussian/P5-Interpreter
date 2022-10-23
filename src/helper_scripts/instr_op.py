import re

file = open('instrs.txt', 'r')
lines = file.readlines()
for line in lines:
    m = re.match(r".*\[\s*(\d+)\]:='(\w+)", line)
    if m is not None:
        print("ins_op(\"{}\", {})".format(m.group(2), m.group(1)))
