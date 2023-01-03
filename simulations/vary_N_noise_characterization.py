import matplotlib.pyplot as plt
import numpy as np
from math import pi, sin, cos

# DFT Constants
F_SAMPLE = 500_000  # Hz
F_SENT = 10_000  # Hz

# Voltage Constants
P_SENT = 600_000_000  # µPa@1m
RECEIVE_GAIN = 0.0000000002320641971  # V/µPa
ESTIMATED_NOISE = 0.0005  # V
CIRCUIT_GAIN = 100  # V / V
MAX_VOLTAGE = 3.3  # V

# Experiment Constants
T_TOTAL = 330  # s

if __name__ == "__main__":
    Ns = [500, 1000, 1250, 2500, 5000]
    Xs = []

    for N in Ns:
        # DFT variables
        K = F_SENT * N / F_SAMPLE
        EXP = np.exp(2j * pi * K / N)

        # Generate data as received
        received = np.random.normal(0, ESTIMATED_NOISE, int(T_TOTAL*F_SAMPLE))
        received *= CIRCUIT_GAIN  # add gain
        received += MAX_VOLTAGE/2  # add offset
        received = np.maximum(np.minimum(received, MAX_VOLTAGE), 0)  # hit the rails

        # Go through ADC
        received = np.round(received / MAX_VOLTAGE * 4096)  # scale and discretize

        # Normalize before Fourier Transform
        received = (received - 2048) / 4096
        received = received.astype(complex)

        # Fourier Transform
        e = np.exp(-2j * pi * K * np.arange(N) / N)
        X = np.convolve(received, e, mode="full")[:received.shape[0]]
        X = np.abs(X) / (N/2)
        Xs.append(X)

    plt.boxplot(Xs, notch=True)
    plt.title("Noise characterization while varying N")
    plt.xticks(range(1, 6), labels=[f"N={N}" for N in Ns])
    plt.grid(which="both")
    plt.xlabel("N")
    plt.ylabel("|X(freq)|")
    plt.show()

    for X in Xs:
        counts, bins = np.histogram(X, 1000)
        plt.stairs(counts, bins)
    plt.legend([f"{N}" for N in Ns], loc='upper left', title='N')
    plt.xlabel("Amplitude")
    plt.ylabel("Count")
    plt.title("Noise characterization while varying N")
    plt.xscale("log")
    plt.grid(which="both")
    plt.show()

    maximums = [np.max(X) for X in Xs]
    minimums = [np.min(X) for X in Xs]
    means = [np.mean(X) for X in Xs]

    plt.plot(Ns, maximums, label="Max")
    plt.plot(Ns, minimums, label="Min")
    plt.plot(Ns, means, label="Means")
    plt.legend()
    plt.xlabel("N")
    plt.ylabel("Amplitude")
    plt.title("Noise characterization while varying N")
    plt.grid(which="both")
    plt.show()
