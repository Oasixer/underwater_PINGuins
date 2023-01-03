import numpy as np
import matplotlib.pyplot as plt
from math import pi


F_SAMPLE = 500_000  # Hz
F_SENT = 10_000  # Hz
N = 1250  # samples
K = F_SENT * N / F_SAMPLE

P_SENT = 600_000_000  # µPa@1m
RECEIVE_GAIN = 0.0000000002320641971  # V/µPa

MAX_VOLTAGE = 3.3  # V


if __name__ == "__main__":
    t = np.arange(0, N/F_SAMPLE, 1/F_SAMPLE)
    ranges = np.arange(1, 50, 0.05)  # m
    received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
    received = np.tile(np.sin(2 * pi * F_SENT * t), [980, 1]) * np.expand_dims(received_voltage, -1)

    e = np.tile(np.exp(-2j*pi * K * np.arange(N) / N), [980, 1])

    gains = [20, 50, 100, 200, 500, 1000]

    Xs = []
    for gain in gains:
        data = received * gain  # add gain
        data += MAX_VOLTAGE / 2  # add offset
        data = np.maximum(np.minimum(data, MAX_VOLTAGE), 0)  # hit the rails

        # Go through ADC
        data = np.round(data / MAX_VOLTAGE * 4096)  # scale and discretize

        # Normalize before Fourier Transform
        data = (data - 2048) / 4096
        data = data.astype(complex)

        # Fourier Transform
        X = np.abs(np.sum(data * e, axis=-1) / (N / 2))
        Xs.append(X)

    plt.figure()
    for X in Xs:
        plt.plot(ranges, X)

    plt.xlabel("range [m]")
    plt.ylabel("Max amplitude")
    plt.title("Max amplitude vs Range at Different gains")
    plt.legend([f"{gain}" for gain in gains])
    plt.yscale("log")
    plt.grid(which="both")
    plt.show()
