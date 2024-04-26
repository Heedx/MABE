"""
File used to grab the handwritten digit datalist from sklearn and apply
the ternary discretization to them. Outputs file named "discrete.txt".

Author: Patrick Ancel
"""

from sklearn.datasets import load_digits
import numpy as np
from statistics import median,mean
import matplotlib.pyplot as plt

metric_lower = lambda x : np.percentile(x, 33)
metric_upper = lambda x : np.percentile(x, 67)

print("Loading Digits...")
digits = load_digits()


print("Discretizing Images...")
# medians = np.zeros(digits.data.shape[0])
# print(len(pixel_lists))
discrete_images = np.zeros(digits.data.shape)

for i in range(digits.data.shape[0]):
    lower = metric_lower(digits.data[i])
    upper = metric_upper(digits.data[i])
    # for j in range(digits.data.shape[1]):
    lower_score = (digits.data[i] > lower).astype(int)
    upper_score = (digits.data[i] > upper).astype(int)
    discrete_images[i,:] = lower_score + upper_score

print("Writing to File...")
# Output results
f = open("discrete.txt", "w")
for t in range(len(discrete_images)):
    f.write(str(digits.target[t]) + "\n")
    image = discrete_images[t].reshape(8, 8)
    for i in range(8):
        for j in range(8):
            f.write(str(int(image[i,j])))
        f.write("\n")
    f.write("\n")
f.close()

# Visualize Images
for i in range(len(discrete_images)):
    image = discrete_images[i].reshape(8, 8).astype(float) / 2
    plt.imshow(image, cmap="gray")
    print(digits.target[i])
    plt.show()
