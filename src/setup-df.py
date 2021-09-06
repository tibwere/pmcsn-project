import os

def setup(destpath):
    times=[i*60 for i in range(1, 9)]

    curdir = os.path.dirname(os.path.abspath(__file__))
    fr = open(curdir + "/../doc/figs/plots/data/raw.csv", "r")
    fw = open(f"{curdir}/../doc/figs/plots/data/{destpath}", "w")

    fw.write("ticket,t,stat,width\n")

    for i in range(6):
        for j in range(len(times)):
            fw.write(f"{i},{times[j]},{fr.readline()}")

    fr.close()
    fw.close()

if __name__ == "__main__":
    setup("d-staz.csv")