import os
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt


def plotter(filename):
    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../plots/data/" + filename + ".csv")

    x = df["seed"]
    y = df["stat"]
    d = df["delta"]

    plt.yticks(np.arange(5, 7, step=0.5))
    plt.ylim(4.5,6.5)
    plt.ylabel("Interval estimate of d[0]")
    plt.xlabel("Initial seed")

    plt.plot(x, y, "ko")
    plt.plot([x, x], [y - d, y + d], color='k', linestyle='-', linewidth=2)
    plt.savefig(curdir + "/../plots/" + filename + ".svg")


if __name__ == "__main__":
    plotter("")