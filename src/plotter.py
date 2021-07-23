import os
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt


def plotter(ticket):
    qos = [15,20,30,90,20,45]

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../doc/figs/plots/data/d-trans.csv")
    df = df[ df["ticket"] == ticket ]

    x = df["M"]
    y = df["stat"]
    w = df["width"]

    plt.xticks(np.arange(2, 6, step=1))
    #plt.ylim(4.5,6.5)
    plt.ylabel("Interval estimate of d[" + str(ticket) +"]")
    plt.xlabel("Number of servers")

    plt.axhline(y=qos[ticket], color='r', linestyle='--', label='QoS')

    plt.plot(x, y, "ko")
    plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)

    plt.legend()

    plt.savefig(curdir + "/../doc/figs/plots/d" + str(ticket) + "-trans.png")
    plt.show()


def get_mean(ticket):
    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../doc/figs/plots/data/d-trans.csv")
    df = df[(df["ticket"] == ticket) & (df["M"] == 4)]

    return df["stat"].iloc[0]


def plot_line(ticket, filename):
    tickets_str = [
        "'Unica Operazione' (BancoPosta)",
        "'Pagamenti & Prelievi' (BancoPosta)",
        "'Unica Operazione' (Standard)",
        "'Pagamenti & Prelievi' (Standard)",
        "'Spedizioni & Ritiri' (BancoPosta)",
        "'Spedizioni & Ritiri' (Standard)"        
    ]
    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(f"{curdir}/../doc/figs/plots/data/{filename}.csv")
    df = df[ df["ticket"] == ticket ]

    get_mean(ticket)

    x = df["t"]
    y = df["stat"]
    w = df["width"]

    plt.xticks(np.arange(0, 500, step=60))
    #plt.ylim(4.5,6.5)
    plt.ylabel(f"Delay of {tickets_str[ticket]} (min)")
    plt.xlabel("Time (min)")

    plt.plot(x, y, "k.")
    plt.plot(x, y)
    plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)
    plt.axhline(y=get_mean(ticket), color='r', linestyle='--', label="mean")

    plt.legend()

    plt.savefig(f"{curdir}/../doc/figs/plots/{filename}-{ticket}.png")
    plt.show()

if __name__ == "__main__":

    for i in range(6):
        plot_line(i, "day-from-mean-values")