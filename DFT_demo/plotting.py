import os
import numpy as np
import matplotlib.pyplot as plt
import imageio

import constants


class Plotter:
    def __init__(self):
        self.frequencies = np.arange(constants.SZ_FREQ_DOMAIN) * constants.F_SAMPLE / constants.SZ_WINDOW

    def plot_frame(self, time, sent, received, freq_domain):
        plt.subplot(3, 1, 1)
        plt.plot(time * 1000, sent)
        plt.ylim(-3.5, 3.5)
        plt.xlabel("Time [ms]")
        plt.ylabel("Amplitude Sent")
        plt.tight_layout()

        plt.subplot(3, 1, 2)
        plt.scatter(time * 1000, received, s=1.0)
        plt.ylim(0, 2**12)
        plt.xlabel("Time [ms]")
        plt.ylabel("Amplitude Received")
        plt.tight_layout()

        plt.subplot(3, 1, 3)
        plt.stem(self.frequencies / 1000, abs(freq_domain), markerfmt=' ', basefmt="-b")
        plt.ylim(0, 500)
        plt.xlim(6_000 / 1000, 30_000 /1000)
        plt.xlabel("Freq [KHz]")
        plt.ylabel("|X(freq)|")
        plt.tight_layout()

    def make_gif(self, filenames):
        with imageio.get_writer('DFT.gif', mode='I') as writer:
            for filename in filenames:
                image = imageio.imread(filename)
                writer.append_data(image)
