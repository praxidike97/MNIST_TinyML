import numpy
import matplotlib.pyplot as plt

from tensorflow.keras.datasets.mnist import load_data


if __name__ == '__main__':
    (x_train, y_train), (x_test, y_test) = load_data()

    plt.imshow(x_train[0], cmap=plt.get_cmap('gray'))
    plt.show()