import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# safe_sum = lambda L, f : f(L[0]) + safe_sum(L[1:], f) if len(L) > 1 else f(L[0])
# print(safe_sum([[1],[2],[3]], lambda x : 2*x))

def safe_sum(L, f):
    total = f(L[0])
    for i in range(1,len(L)):
        total += f(L[i])
    return total

def safe_avg(L, f):
    total = safe_sum(L, f)
    return total / len(L)

markov_retina_2_nodes_8_folder = "C0__BT_Markov__RT_2__HID_8"
markov_retina_2_nodes_16_folder = "C1__BT_Markov__RT_2__HID_16"
rnn_retina_2_nodes_8_folder = "C2__BT_RNN__RT_2__NRN_8"
rnn_retina_2_nodes_16_folder = "C3__BT_RNN__RT_2__NRN_16"

markov_retina_3_nodes_8_folder = "C0__BT_Markov__RT_3__HID_8"
markov_retina_3_nodes_16_folder = "C1__BT_Markov__RT_3__HID_16"
rnn_retina_3_nodes_8_folder = "C2__BT_RNN__RT_3__NRN_8"
rnn_retina_3_nodes_16_folder = "C3__BT_RNN__RT_3__NRN_16"

def get_data_from_folder(folder):
    data = []
    for i in range(1,11):
        filename = F"./{folder}/{100 + i}/max.csv"
        data.append(pd.read_csv(filename))
    return data

if __name__ == "__main__":
    data_markov_retina_2_nodes_8 = get_data_from_folder(markov_retina_2_nodes_8_folder)
    data_markov_retina_2_nodes_16 = get_data_from_folder(markov_retina_2_nodes_16_folder)
    data_rnn_retina_2_nodes_8 = get_data_from_folder(rnn_retina_2_nodes_8_folder)
    data_rnn_retina_2_nodes_16 = get_data_from_folder(rnn_retina_2_nodes_16_folder)

    data_markov_retina_3_nodes_8 = get_data_from_folder(markov_retina_3_nodes_8_folder)
    data_markov_retina_3_nodes_16 = get_data_from_folder(markov_retina_3_nodes_16_folder)
    data_rnn_retina_3_nodes_8 = get_data_from_folder(rnn_retina_3_nodes_8_folder)
    data_rnn_retina_3_nodes_16 = get_data_from_folder(rnn_retina_3_nodes_16_folder)


    R_markov_retina_2_nodes_8 = safe_avg(data_markov_retina_2_nodes_8, lambda x : x["R"])
    earlyR25_markov_retina_2_nodes_8 = safe_avg(data_markov_retina_2_nodes_8, lambda x : x["earlyR25"])
    lateR25_markov_retina_2_nodes_8 = safe_avg(data_markov_retina_2_nodes_8, lambda x : x["lateR25"])

    R_markov_retina_2_nodes_16 = safe_avg(data_markov_retina_2_nodes_16, lambda x : x["R"])
    earlyR25_markov_retina_2_nodes_16 = safe_avg(data_markov_retina_2_nodes_16, lambda x : x["earlyR25"])
    lateR25_markov_retina_2_nodes_16 = safe_avg(data_markov_retina_2_nodes_16, lambda x : x["lateR25"])

    R_rnn_retina_2_nodes_8 = safe_avg(data_rnn_retina_2_nodes_8, lambda x : x["R"])
    earlyR25_rnn_retina_2_nodes_8 = safe_avg(data_rnn_retina_2_nodes_8, lambda x : x["earlyR25"])
    lateR25_rnn_retina_2_nodes_8 = safe_avg(data_rnn_retina_2_nodes_8, lambda x : x["lateR25"])

    R_rnn_retina_2_nodes_16 = safe_avg(data_rnn_retina_2_nodes_16, lambda x : x["R"])
    earlyR25_rnn_retina_2_nodes_16 = safe_avg(data_rnn_retina_2_nodes_16, lambda x : x["earlyR25"])
    lateR25_rnn_retina_2_nodes_16 = safe_avg(data_rnn_retina_2_nodes_16, lambda x : x["lateR25"])


    R_markov_retina_3_nodes_8 = safe_avg(data_markov_retina_3_nodes_8, lambda x : x["R"])
    earlyR25_markov_retina_3_nodes_8 = safe_avg(data_markov_retina_3_nodes_8, lambda x : x["earlyR25"])
    lateR25_markov_retina_3_nodes_8 = safe_avg(data_markov_retina_3_nodes_8, lambda x : x["lateR25"])

    R_markov_retina_3_nodes_16 = safe_avg(data_markov_retina_3_nodes_16, lambda x : x["R"])
    earlyR25_markov_retina_3_nodes_16 = safe_avg(data_markov_retina_3_nodes_16, lambda x : x["earlyR25"])
    lateR25_markov_retina_3_nodes_16 = safe_avg(data_markov_retina_3_nodes_16, lambda x : x["lateR25"])

    R_rnn_retina_3_nodes_8 = safe_avg(data_rnn_retina_3_nodes_8, lambda x : x["R"])
    earlyR25_rnn_retina_3_nodes_8 = safe_avg(data_rnn_retina_3_nodes_8, lambda x : x["earlyR25"])
    lateR25_rnn_retina_3_nodes_8 = safe_avg(data_rnn_retina_3_nodes_8, lambda x : x["lateR25"])

    R_rnn_retina_3_nodes_16 = safe_avg(data_rnn_retina_3_nodes_16, lambda x : x["R"])
    earlyR25_rnn_retina_3_nodes_16 = safe_avg(data_rnn_retina_3_nodes_16, lambda x : x["earlyR25"])
    lateR25_rnn_retina_3_nodes_16 = safe_avg(data_rnn_retina_3_nodes_16, lambda x : x["lateR25"])


    plt.plot(data_markov_retina_2_nodes_8[0]["update"], R_markov_retina_2_nodes_8, label="Markov Brain")
    plt.plot(data_rnn_retina_2_nodes_8[0]["update"], R_rnn_retina_2_nodes_8, label="RNN")

    plt.legend()
    plt.title("2x2 Eye 8-Node R")
    plt.xlabel("Generation")
    plt.ylabel("R")
    plt.savefig("Average 2x2-Eye 8-Node - R.png")

    plt.clf()

    plt.plot(data_markov_retina_2_nodes_16[0]["update"], R_markov_retina_2_nodes_16, label="Markov Brain")
    plt.plot(data_rnn_retina_2_nodes_16[0]["update"], R_rnn_retina_2_nodes_16, label="RNN")

    plt.legend()
    plt.title("2x2 Eye 16-Node R")
    plt.xlabel("Generation")
    plt.ylabel("R")
    plt.savefig("Average 2x2-Eye 16-Node - R.png")

    plt.clf()

    plt.plot(data_markov_retina_2_nodes_8[0]["update"], earlyR25_markov_retina_2_nodes_8, label="Markov Brain Early")
    plt.plot(data_markov_retina_2_nodes_8[0]["update"], lateR25_markov_retina_2_nodes_8, label="Markov Brain Late")
    plt.plot(data_rnn_retina_2_nodes_8[0]["update"], earlyR25_rnn_retina_2_nodes_8, label="RNN Early")
    plt.plot(data_rnn_retina_2_nodes_8[0]["update"], lateR25_rnn_retina_2_nodes_8, label="RNN Late")

    plt.legend()
    plt.title("2x2 Eye 8-Node Early vs. Late R")
    plt.xlabel("Generation")
    plt.ylabel("R")
    plt.savefig("Average 2x2-Eye 8-Node - Early Late R.png")

    plt.clf()


    plt.plot(data_markov_retina_3_nodes_8[0]["update"], R_markov_retina_3_nodes_8, label="Markov Brain")
    plt.plot(data_rnn_retina_3_nodes_8[0]["update"], R_rnn_retina_3_nodes_8, label="RNN")

    plt.legend()
    plt.title("3x3 Eye 8-Node R")
    plt.xlabel("Generation")
    plt.ylabel("R")
    plt.savefig("Average 3x3-Eye 8-Node - R.png")

    plt.clf()


    plt.plot(data_markov_retina_3_nodes_16[0]["update"], R_markov_retina_3_nodes_16, label="Markov Brain")
    plt.plot(data_rnn_retina_3_nodes_16[0]["update"], R_rnn_retina_3_nodes_16, label="RNN")

    plt.legend()
    plt.title("3x3 Eye 16-Node R")
    plt.xlabel("Generation")
    plt.ylabel("R")
    plt.savefig("Average 3x3-Eye 16-Node - R.png")

    plt.clf()


    plt.plot(data_markov_retina_3_nodes_8[0]["update"], earlyR25_markov_retina_3_nodes_8, label="Markov Brain Early")
    plt.plot(data_markov_retina_3_nodes_8[0]["update"], lateR25_markov_retina_3_nodes_8, label="Markov Brain Late")
    plt.plot(data_rnn_retina_3_nodes_8[0]["update"], earlyR25_rnn_retina_3_nodes_8, label="RNN Early")
    plt.plot(data_rnn_retina_3_nodes_8[0]["update"], lateR25_rnn_retina_3_nodes_8, label="RNN Late")

    plt.legend()
    plt.title("3x3 Eye 8-Node Early vs. Late R")
    plt.xlabel("Generation")
    plt.ylabel("R")
    plt.savefig("Average 3x3-Eye 8-Node - Early Late R.png")

    plt.clf()