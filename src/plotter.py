import os
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt


def plotter(index):
    qos = [12.439024,18.6585366,12.439024*3,18.6585366*8,24.878049,24.878049*3]

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../doc/figs/plots/data/d" + str(index) + "-trans.csv")

    x = df["M"]
    y = df["stat"]
    w = df["width"]

    plt.xticks(np.arange(2, 6, step=1))
    #plt.ylim(4.5,6.5)
    plt.ylabel("Interval estimate of d[" + str(index) +"]")
    plt.xlabel("Number of servers")

    plt.axhline(y=qos[index], color='r', linestyle='--', label='QoS')

    plt.plot(x, y, "ko")
    plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)

    plt.legend()

    plt.savefig(curdir + "/../doc/figs/plots/d" + str(index) + "-trans.png")
    plt.show()

def plotter_queue(index):
    qos = [1,2,3,8,2,3]

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../doc/figs/plots/q" + str(index) + "-trans.csv")

    x = df["M"]
    y = df["stat"]
    w = df["width"]

    plt.xticks(np.arange(2, 6, step=1))
    #plt.ylim(4.5,6.5)
    plt.ylabel("Interval estimate of q[" + str(index) +"]")
    plt.xlabel("Number of servers")

    plt.axhline(y=qos[index], color='r', linestyle='--', label='QoS')

    plt.plot(x, y, "ko")
    plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)

    plt.legend()

    plt.savefig(curdir + "/../doc/figs/plots/q" + str(index) + "-trans.png")
    plt.show()

if __name__ == "__main__":
    for i in range(6):
        plotter(i)