import json
import sys
import matplotlib.pyplot as plt
f = open(sys.argv[1], "r")
j = json.load(f)
for lit in j.keys():
    lit_int = int(lit)
    xs = []
    ys = []
    for pair in j[lit]:
        xs.append(pair[0])
        ys.append(pair[1])
    plt.plot(xs, ys)

plt.show()

