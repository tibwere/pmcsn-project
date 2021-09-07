#!/usr/bin/python3
import os
from matplotlib.transforms import Transform
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

tickets_str = [
    "'Unica Operazione BancoPosta'",
    "'Pagamenti & Prelievi BancoPosta'",
    "'Unica Operazione Standard'",
    "'Pagamenti & Prelievi Standard'",
    "'Spedizioni & Ritiri BancoPosta'",
    "'Spedizioni & Ritiri Standard'"        
]


def plot_delay_M(ticket):
    qos = [5,7.5,10,12.5,10,15]

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../doc/figs/plots/data/d-trans.csv")
    #df = pd.read_csv(curdir + "/../doc/figs/plots/data/d-trans-imp.csv")
    df = df[ df["ticket"] == ticket ]

    x = df["M"]
    y = df["stat"]
    w = df["width"]

    plt.xticks(np.arange(1, 5, step=1))
    plt.title(f"Intervallo di confidenza per l'attesa media della classe:\n{tickets_str[ticket]}")
    plt.ylabel(f"Attesa media (min)")
    plt.xlabel("Numero di server (M)")

    plt.axhline(y=qos[ticket], color='r', linestyle='--', label='QoS')
    plt.text(
        # 4.8,
        3.8,
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
    #plt.savefig(curdir + "/../doc/figs/plots/d" + str(ticket) + "-trans-imp.png")
    plt.show()

def get_mean(ticket):
    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(curdir + "/../doc/figs/plots/data/d-trans.csv")
    df = df[(df["ticket"] == ticket) & (df["M"] == 4)]

    return df["stat"].iloc[0]

def plot_delay_t_terminating(ticket, need_mean=False):
    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(f"{curdir}/../doc/figs/plots/data/day-from-empty.csv")
    df = df[ df["ticket"] == ticket ]

    x = df["t"]
    y = df["stat"]
    w = df["width"]
    
    plt.xticks(np.arange(0, 500, step=60))

    plt.title(f"Attesa media della classe {tickets_str[ticket]}\nal variare dell'ampiezza dell'intervallo temporale considerato")
    plt.ylabel(f"Attesa media (min)")
    plt.xlabel("Tempo (min)")

    plt.plot(x, y, "k.")
    plt.plot(x, y, label="Attesa al variare dell'istante di campionamento")
    plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)

    if need_mean:
        mean = get_mean(ticket)
        plt.axhline(y=mean, color='r', linestyle='--', label="Media giornaliera")
        plt.text(
            60,
            mean,
            str(mean),
            fontsize=10,
            va="center",
            ha="center",
            color="red",
            backgroundcolor="white"
        )

        plt.legend(loc="lower right")

    plt.savefig(f"{curdir}/../doc/figs/plots/d{ticket}-no-stat.png")
    plt.show()

def plot_delay_t_terminating_all():

    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(f"{curdir}/../doc/figs/plots/data/day-from-empty.csv")
    
    plt.figure(figsize=(16,9))
    plt.xticks(np.arange(0, 500, step=60))

    plt.title(f"Attesa media delle classi \nal variare dell'ampiezza dell'intervallo temporale considerato")
    plt.ylabel(f"Attesa media (min)")
    plt.xlabel("Tempo (min)")
    
    for i in range(6):
        sample_df = df[ df["ticket"] == i ]

        x = sample_df["t"]
        y = sample_df["stat"]
        w = sample_df["width"]

        plt.plot(x, y, "k.")
        plt.plot(x, y, label=tickets_str[i])
        plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)

    plt.legend(loc="upper left", fontsize="medium")

    plt.savefig(f"{curdir}/../doc/figs/plots/day-from-empty-all.png")
    plt.show()

def plot_delay_t_no_stationary():
    curdir = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(f"{curdir}/../doc/figs/plots/data/d2-no-stat.csv")
    x = df["t"]
    y = df["stat"]
    w = df["width"]
    
    plt.xticks(np.arange(60, 1020, step=180))

    plt.title(f"Attesa media della classe {tickets_str[2]}\nal variare dell'ampiezza dell'intervallo temporale considerato")
    plt.ylabel(f"Attesa media (min)")
    plt.xlabel("Tempo (min)")

    plt.plot(x, y, "k.")
    plt.plot(x, y, label="Attesa al variare dell'istante di campionamento")
    plt.plot([x, x], [y - w, y + w], color='k', linestyle='-', linewidth=2)
    plt.axhline(y=y.iloc[8], color='r', linestyle='-.', label="Attesa campionata a 480 minuti")
    plt.legend(loc="lower right")

    plt.savefig(f"{curdir}/../doc/figs/plots/d2-no-stat.png")
    plt.show()

if __name__ == "__main__":

    for i in range(6):
        plot_delay_M(i)

    # for i in range(6):
    #     plot_delay_t_terminating(i, need_mean=False)

    # plot_delay_t_terminating_all()

    # plot_delay_t_stationary('G')
    # plot_delay_t_stationary('D')


    # plot_delay_t_no_stationary()
