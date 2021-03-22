import csv
import subprocess

import string

import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
    with open('vystup.csv', newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',', quotechar='"')
        header = next(reader)
        data = list(reader)


    podle_ceho = 1 #index 1 = podle expersions
    values = set(map(lambda x:x[podle_ceho],data))
    newlist = [[y for y in data if y[podle_ceho] == x] for x in values]

    expresions = [x[0][1] for x in newlist]
    time_p = [float(x[0][5]) for x in newlist]
    time_c = [float(x[1][5]) for x in newlist]
    plt.bar(expresions, time_p)
    plt.bar(expresions, time_c, align='edge')

    plt.savefig("test.png")

    print("\n")

    #subprocess.call() #dokáže zavolat gcc nebo mělo by i make


