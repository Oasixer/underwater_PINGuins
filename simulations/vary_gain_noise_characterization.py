import numpy as np
import matplotlib.pyplot as plt
from math import pi

# DFT Constants
F_SAMPLE = 500_000  # Hz
F_SENT = 10_000  # Hz
N = 1250  # samples
K = F_SENT * N / F_SAMPLE
EXP = np.exp(2j*pi * K / N)

# Voltage Constants
P_SENT = 600_000_000  # µPa@1m
RECEIVE_GAIN = 0.0000000002320641971  # V/µPa
ESTIMATED_NOISE = 0.0005  # V
MAX_VOLTAGE = 3.3  # V

# Experiment Constants
T_TOTAL = 330  # s

if __name__ == "__main__":
    gains = np.array([20, 50, 100, 500, 1000])

    # Create data
    t = np.arange(0, T_TOTAL, 1 / F_SAMPLE)

    # Go through amplifier circuit
    received = np.random.normal(0, ESTIMATED_NOISE, [gains.shape[0], t.shape[0]])  # add Gaussian noise
    received *= np.expand_dims(gains, axis=-1)  # add gain
    received += MAX_VOLTAGE/2  # add offset
    received = np.maximum(np.minimum(received, MAX_VOLTAGE), 0)  # hit the rails

    # Go through ADC
    received = np.round(received / MAX_VOLTAGE * 4096)  # scale and discretize

    # Normalize before Fourier Transform
    received = (received - 2048) / 4096
    received = received.astype(complex)

    # Fourier Transform
    e = np.exp(-2j * pi * K * np.arange(N) / N)
    Xs = np.apply_along_axis(lambda m: np.convolve(m, e, mode='full'), axis=-1, arr=received)[:, :received.shape[1]]
    Xs = np.abs(Xs) / (N / 2)

    data = [X for X in Xs]
    plt.boxplot(data, notch=True)
    plt.title("Noise characterization while varying gain")
    plt.xticks(range(1, 6), labels=[str(gain) for gain in gains])
    plt.grid(which="both")
    plt.xlabel("Gain")
    plt.ylabel("|X(freq)|")
    plt.show()

    for X in Xs:
        counts, bins = np.histogram(X, 1000)
        plt.stairs(counts, bins)
    plt.legend([f"{gain}" for gain in gains], loc='upper left', title="gain")
    plt.xlabel("Amplitude")
    plt.ylabel("Count")
    plt.title("Noise characterization while varying gain")
    plt.xscale("log")
    plt.grid(which="both")
    plt.show()

    maximums = [np.max(X) for X in Xs]
    minimums = [np.min(X) for X in Xs]
    means = [np.mean(X) for X in Xs]

    plt.plot(gains, maximums, label="Max")
    plt.plot(gains, minimums, label="Min")
    plt.plot(gains, means, label="Means")
    plt.legend()
    plt.xlabel("gain")
    plt.ylabel("Amplitude")
    plt.title("Noise characterization while varying gain")
    plt.grid(which="both")
    plt.show()
