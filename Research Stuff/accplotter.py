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
    
    totalCorrect_markov_retina_2_nodes_8 = safe_sum(data_markov_retina_2_nodes_8, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_markov_retina_2_nodes_8 = safe_sum(data_markov_retina_2_nodes_8, lambda data : data["totalIncorrect_AVE"])

    totalCorrect_markov_retina_2_nodes_16 = safe_sum(data_markov_retina_2_nodes_16, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_markov_retina_2_nodes_16 = safe_sum(data_markov_retina_2_nodes_16, lambda data : data["totalIncorrect_AVE"])

    totalCorrect_rnn_retina_2_nodes_8 = safe_sum(data_rnn_retina_2_nodes_8, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_rnn_retina_2_nodes_8 = safe_sum(data_rnn_retina_2_nodes_8, lambda data : data["totalIncorrect_AVE"])

    totalCorrect_rnn_retina_2_nodes_16 = safe_sum(data_rnn_retina_2_nodes_16, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_rnn_retina_2_nodes_16 = safe_sum(data_rnn_retina_2_nodes_16, lambda data : data["totalIncorrect_AVE"])



    totalCorrect_markov_retina_3_nodes_8 = safe_sum(data_markov_retina_3_nodes_8, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_markov_retina_3_nodes_8 = safe_sum(data_markov_retina_3_nodes_8, lambda data : data["totalIncorrect_AVE"])

    totalCorrect_markov_retina_3_nodes_16 = safe_sum(data_markov_retina_3_nodes_16, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_markov_retina_3_nodes_16 = safe_sum(data_markov_retina_3_nodes_16, lambda data : data["totalIncorrect_AVE"])

    totalCorrect_rnn_retina_3_nodes_8 = safe_sum(data_rnn_retina_3_nodes_8, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_rnn_retina_3_nodes_8 = safe_sum(data_rnn_retina_3_nodes_8, lambda data : data["totalIncorrect_AVE"])

    totalCorrect_rnn_retina_3_nodes_16 = safe_sum(data_rnn_retina_3_nodes_16, lambda data : data["totalCorrect_AVE"])
    totalIncorrect_rnn_retina_3_nodes_16 = safe_sum(data_rnn_retina_3_nodes_16, lambda data : data["totalIncorrect_AVE"])


    plt.plot(data_markov_retina_2_nodes_8[0]['update'], totalCorrect_markov_retina_2_nodes_8 / (totalCorrect_markov_retina_2_nodes_8 + totalIncorrect_markov_retina_2_nodes_8), label="Markov Brain")
    plt.plot(data_rnn_retina_2_nodes_8[0]['update'], totalCorrect_rnn_retina_2_nodes_8 / (totalCorrect_rnn_retina_2_nodes_8 + totalIncorrect_rnn_retina_2_nodes_8), label="RNN")
    
    plt.legend()
    plt.title('2x2 Eye 8-Node Accuracy')
    plt.xlabel('Generation')
    plt.ylabel('Accuracy')
    plt.savefig('Average 2x2-Eye 8-Node - correct over total.png')

    plt.clf()

    plt.plot(data_markov_retina_2_nodes_16[0]['update'], totalCorrect_markov_retina_2_nodes_16 / (totalCorrect_markov_retina_2_nodes_16 + totalIncorrect_markov_retina_2_nodes_16), label="Markov Brain")
    plt.plot(data_rnn_retina_2_nodes_16[0]['update'], totalCorrect_rnn_retina_2_nodes_16 / (totalCorrect_rnn_retina_2_nodes_16 + totalIncorrect_rnn_retina_2_nodes_16), label="RNN")
    
    plt.legend()
    plt.title('2x2 Eye 16-Node Accuracy')
    plt.xlabel('Generation')
    plt.ylabel('Accuracy')
    plt.savefig('Average 2x2-Eye 16-Node - correct over total.png')

    plt.clf()


    plt.plot(data_markov_retina_3_nodes_8[0]['update'], totalCorrect_markov_retina_3_nodes_8 / (totalCorrect_markov_retina_3_nodes_8 + totalIncorrect_markov_retina_3_nodes_8), label="Markov Brain")
    plt.plot(data_rnn_retina_3_nodes_8[0]['update'], totalCorrect_rnn_retina_3_nodes_8 / (totalCorrect_rnn_retina_3_nodes_8 + totalIncorrect_rnn_retina_3_nodes_8), label="RNN")
    
    plt.legend()
    plt.title('3x3 Eye 8-Node Accuracy')
    plt.xlabel('Generation')
    plt.ylabel('Accuracy')
    plt.savefig('Average 3x3-Eye 8-Node - correct over total.png')

    plt.clf()


    plt.plot(data_markov_retina_3_nodes_16[0]['update'], totalCorrect_markov_retina_3_nodes_16 / (totalCorrect_markov_retina_3_nodes_16 + totalIncorrect_markov_retina_3_nodes_16), label="Markov Brain")
    plt.plot(data_rnn_retina_3_nodes_16[0]['update'], totalCorrect_rnn_retina_3_nodes_16 / (totalCorrect_rnn_retina_3_nodes_16 + totalIncorrect_rnn_retina_3_nodes_16), label="RNN")
    
    plt.legend()
    plt.title('3x3 Eye 16-Node Accuracy')
    plt.xlabel('Generation')
    plt.ylabel('Accuracy')
    plt.savefig('Average 3x3-Eye 16-Node - correct over total.png')

    plt.clf()