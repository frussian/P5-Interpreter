import re

file = open('sptable.txt', 'r')
lines = file.readlines()
for line in lines:
    splitted = line.split(';')
    for spl in splitted:
        m = re.match(r".*\[\s*(\d+)\]:='(\w+)", spl)
        if m is None:
            continue
        print("sp_table_op(\"{}\", {})".format(m.group(2), m.group(1)))
