import csv
import subprocess
import os

import string

import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":

    path = os.getcwd()
    path_parent = os.path.dirname(os.getcwd())

    num_of_test_runs = 5

    for i in range(1,num_of_test_runs+1):
        cporcess = subprocess.run(["./test_speed_bin", f"test{i}.csv"], cwd=os.path.join(path_parent, "build"), )  # dokáže zavolat gcc nebo mělo by i make
        os.replace(os.path.join(path_parent, "build", f"test{i}.csv"), os.path.join(path_parent, "test", f"test{i}.csv"))
    print("\n")

    files = [f"test{i}.csv" for i in range(1,num_of_test_runs+1)]

    data = []

    for f in files:
        with open(f, newline='') as csvfile:
            reader = csv.reader(csvfile, delimiter=',', quotechar='"')
            header = next(reader)
            data += list(reader)


    podle_ceho = 1 #index 1 = podle expersions
    values = set(map(lambda x:x[podle_ceho],data))
    newlist = [[y for y in data if y[podle_ceho] == x] for x in values]

    expresions = [x[0][1] for x in newlist]
    time_p = [float(x[0][5]) for x in newlist]
    time_c = [float(x[1][5]) for x in newlist]
    plt.bar(expresions, time_p)
    plt.bar(expresions, time_c, align='edge')

    plt.savefig("test.png")



