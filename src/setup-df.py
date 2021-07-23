def setup(destpath):
    times=[45, 47, 49, 53, 61, 77, 109, 173, 301, 480]

    fr = open("../doc/figs/plots/data/raw.csv", "r")
    fw = open(f"../doc/figs/plots/data/{destpath}", "w")

    fw.write("ticket,t,stat,width\n")

    for i in range(6):
        for j in range(len(times)):
            fw.write(f"{i},{times[j]},{fr.readline()}")

    fr.close()
    fw.close()

if __name__ == "__main__":
    setup("day-from-mean-values.csv")