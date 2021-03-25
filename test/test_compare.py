import csv
import string

import subprocess
import os

import pandas as pd
import numpy as np
import plotly.express as px

if __name__ == "__main__":

    run_tests_switch = False
    n_repeats = 1000
    path = os.getcwd()
    path_parent = os.path.dirname(os.getcwd())

    num_of_test_runs = 5

    files = []

    for i in range(1, num_of_test_runs + 1):
        #BParser
        if run_tests_switch:
            cporcess = subprocess.run(["./test_speed_parser_bin", f"test_parser{i}.csv", str(n_repeats)], cwd=os.path.join(path_parent, "build"), )  # dokáže zavolat gcc nebo mělo by i make
            os.replace(os.path.join(path_parent, "build", f"test_parser{i}.csv"), os.path.join(path_parent, "test", f"test_parser{i}.csv"))
        
        files.append(os.path.join(path_parent, "test", f"test_parser{i}.csv"))
        
        #C++
        if run_tests_switch:
            cporcess = subprocess.run(["./test_speed_cpp_bin", f"test_cpp{i}.csv", str(n_repeats)], cwd=os.path.join(path_parent, "build"), )  # dokáže zavolat gcc nebo mělo by i make
            os.replace(os.path.join(path_parent, "build", f"test_cpp{i}.csv"), os.path.join(path_parent, "test", f"test_cpp{i}.csv"))
        
        files.append(os.path.join(path_parent, "test", f"test_cpp{i}.csv"))

    list_of_dataframes = []
    for f in files:
        with open(f, newline='') as csvfile:
            list_of_dataframes.append(pd.read_csv(f))
    data = pd.concat(list_of_dataframes)

    #data_sort_by_expressions = data.sort_values(by=['Expression'])
    #data_sort_by_expressions_and_executor = data.sort_values(by=['Expression', 'Executor'])

    data_med = data.groupby(['Executor', 'ID', 'Expression'], as_index=False)['Time'].median()

    title_string = "Expression by time execution for " + str(data['Repeats'].values[0]) + " repeats"
    #Figure
    fig = px.scatter(data_med, x="Time", y="Expression", color="Executor",
                 title=title_string,
                 labels={"Time":"Time (s)", "Expression":"Expressions"} # customize axis label
                )

    fig.show()


    #expresions = [x[0][1] for x in list_by_experesions]
    #time_p = [float(x[0][5]) for x in newlist]
    #time_c = [float(x[1][5]) for x in newlist]

