import os

import numpy as np
from numpy import pi
import matplotlib.pyplot as plt
import imageio

import constants
from message_generator import bits_to_amplitude, add_noise, demo_amplitude_generator, make_integers
from sliding_DFT import SlidingDFT
from plotting import Plotter


def main():
    msg = [[10_000], [18_000], [26_000], [10_000, 18_000, 26_000]]

    t, sent = demo_amplitude_generator(msg)
    received = add_noise(np.copy(sent))
    sampled = make_integers(received, 12)

    sliding_dft = SlidingDFT()
    plotter = Plotter()

    save_directory = "frames/"
    file_names = []

    max_val = 0
    for i in range(t.shape[0]-1):
        sliding_dft.update(sampled[i])

        curr_max_val = max(sliding_dft.freq_domain)
        if curr_max_val > max_val:
            max_val = curr_max_val

        if i >= 1250 and i % 1250 == 0:
            plotter.plot_frame(t[i-1250:i], sent[i-1250:i], sampled[i-1250:i], sliding_dft.freq_domain / constants.SZ_FREQ_DOMAIN)
            plt.savefig(f"{save_directory}{i}.png")
            plt.close()
            print(i)
    print(max_val)


if __name__ == "__main__":
    main()
    # filenames = [f"frames/{i}.png" for i in range(1250, 21250, 50)]
    # print(filenames)
    # images = []
    # for filename in filenames:
    #     images.append(imageio.imread(filename))
    # print("got images")
    # imageio.mimsave("sampled_dft.gif", images, duration=0.001)
    # print("done")
