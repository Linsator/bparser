import csv
import subprocess
import os

import string


import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":

    path = os.getcwd()
    path_parent = os.path.dirname(os.getcwd())

    num_of_test_runs = 5
    files = []

    for i in range(1,num_of_test_runs+1):
        #cporcess = subprocess.run(["./test_speed_parser_bin", f"test_parser{i}.csv"], cwd=os.path.join(path_parent, "build"), )  # dokáže zavolat gcc nebo mělo by i make
        #os.replace(os.path.join(path_parent, "build", f"test_parser{i}.csv"), os.path.join(path_parent, "test", f"test_parser{i}.csv"))
        files.append(os.path.join(path_parent, "test", f"test_parser{i}.csv"))
        #cporcess = subprocess.run(["./test_speed_cpp_bin", f"test_cpp{i}.csv"], cwd=os.path.join(path_parent, "build"), )  # dokáže zavolat gcc nebo mělo by i make
        #os.replace(os.path.join(path_parent, "build", f"test_cpp{i}.csv"), os.path.join(path_parent, "test", f"test_cpp{i}.csv"))
        files.append(os.path.join(path_parent, "test", f"test_cpp{i}.csv"))

    list_of_dataframes = []
    for f in files:
        with open(f, newline='') as csvfile:
            list_of_dataframes.append(pd.read_csv(f))
    data = pd.concat(list_of_dataframes)


    expresions = [x[0][1] for x in list_by_experesions]
    time_p = [float(x[0][5]) for x in newlist]
    time_c = [float(x[1][5]) for x in newlist]
    plt.bar(expresions, time_p)
    plt.bar(expresions, time_c, align='edge')

    plt.savefig("test.png")



