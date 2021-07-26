#!/usr/bin/python3
import os
from matplotlib.transforms import Transform
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

tickets_str = [
    "'Unica Operazione' (BancoPosta)",
    "'Pagamenti & Prelievi' (BancoPosta)",
    "'Unica Operazione' (Standard)",
    "'Pagamenti & Prelievi' (Standard)",
    "'Spedizioni & Ritiri' (BancoPosta)",
    "'Spedizioni & Ritiri' (Standard)"        
]


def plot_delay_M(ticket):
    qos = [10,15,45,120,20,45]

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../doc/figs/plots/data/d-trans.csv")
    df = df[ df["ticket"] == ticket ]

    x = df["M"]
    y = df["stat"]
    w = df["width"]

    plt.xticks(np.arange(2, 6, step=1))
    plt.title(f"Intervallo di confidenza per l'attesa media di:\n{tickets_str[ticket]}")
    plt.ylabel(f"Attesa media (min)")
    plt.xlabel("Numero di server (M)")

    plt.axhline(y=qos[ticket], color='r', linestyle='--', label='QoS')
    plt.text(
        4.8,
        qos[ticket],
        str(qos[ticket]),
        fontsize=10,
        va="center",
        ha="center",
        color="red",
        backgroundcolor="white"
    )

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


def plot_delay_t_terminating(ticket, filename, xtext):

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(f"{curdir}/../doc/figs/plots/data/{filename}.csv")
    df = df[ df["ticket"] == ticket ]

    get_mean(ticket)

    x = df["t"]
    y = df["stat"]
    w = df["width"]
    mean = get_mean(ticket)

    plt.xticks(np.arange(0, 500, step=60))

    plt.title(f"Attesa media di {tickets_str[ticket]}\nal variare dell'ampiezza dell'intervallo temporale considerato")
    plt.ylabel(f"Attesa media (min)")
    plt.xlabel("Tempo (min)")

    plt.plot(x, y, "k.")
    plt.plot(x, y)
    plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)
    plt.axhline(y=mean, color='r', linestyle='--', label="mean")
    plt.text(
        xtext,
        mean,
        str(mean),
        fontsize=10,
        va="center",
        ha="center",
        color="red",
        backgroundcolor="white"
    )
    
    plt.legend()

    plt.savefig(f"{curdir}/../doc/figs/plots/{filename}-{ticket}.png")
    plt.show()

def plot_delay_t_stationary(type, show_interval=False):
    type_str = [
        "'Unica Operazione' e 'Pagamenti & Prelievi'",
        "'Spedizioni & Ritiri'"        
    ]
    x_ticks = np.array([1,8,16,32,64,128,256])
    index = 0 if type == 'G' else 1

    plt.figure(figsize=(16,9))

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(f"{curdir}/../doc/figs/plots/data/d-staz.csv")
    df = df[ df["type"] == type ]

    x = df["nbatch"]
    y = df["stat"]
    w = df["width"]

    plt.xticks(x_ticks)

    plt.title(f"Raggiungimento della stazionarietà dell'attesa media di:\n{type_str[index]}")
    plt.ylabel(f"Attesa media (min)")
    plt.xlabel("Numero di batches (K)")

    plt.plot(x, y, "ko")
    plt.plot(x, y)

    if show_interval:
        plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)

    if show_interval:
        plt.savefig(f"{curdir}/../doc/figs/plots/d-staz-{type}-ic.png")
    else:
        plt.savefig(f"{curdir}/../doc/figs/plots/d-staz-{type}.png")

    plt.show()   

if __name__ == "__main__":

    # for i in range(6):
    #     plot_delay_M(i)

    # for i in range(6):
    #     plot_delay_t_terminating(i, "day-from-empty", 60)

    # for i in range(6):
    #     plot_delay_t_terminating(i, "day-from-mean-values", 460)

    # plot_delay_t_stationary('G')
    # plot_delay_t_stationary('D')

    plot_delay_t_stationary('G', show_interval=True)
    plot_delay_t_stationary('D', show_interval=True)
