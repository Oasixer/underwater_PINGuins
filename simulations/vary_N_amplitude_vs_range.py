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
T_TOTAL = 3.3  # s
T_ACTIVE_START = 1.0  # s
I_ACTIVE_START = int(F_SAMPLE / T_ACTIVE_START)
N_EXPERIMENTS = 1000

if __name__ == "__main__":
    ranges = np.arange(1, 50, 0.05)  # m
    Ns = [500, 1000, 1250, 2500, 5000]
    Xs = []
    for N in Ns:
        # DFT variables
        K = F_SENT * N / F_SAMPLE
        EXP = np.exp(2j * pi * K / N)

        # Generate data as received
        t = np.arange(0, N/F_SAMPLE, 1/F_SAMPLE)[:N]
        ranges = np.arange(1, 50, 0.05)  # m
        received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
        received = np.tile(np.sin(2 * pi * F_SENT * t), [980, 1]) * np.expand_dims(received_voltage, -1)

        data = received * CIRCUIT_GAIN  # add gain
        data += MAX_VOLTAGE / 2  # add offset
        # Go through ADC
        data = np.round(data / MAX_VOLTAGE * 4096)  # scale and discretize

        # Normalize before Fourier Transform
        data = (data - 2048) / 4096
        data = data.astype(complex)

        # Fourier Transform
        e = np.tile(np.exp(-2j * pi * K * np.arange(N) / N), [980, 1])
        X = np.abs(np.sum(data * e, axis=-1) / (N / 2))
        Xs.append(X)

    plt.figure()
    for N, X in zip(Ns, Xs):
        plt.plot(ranges, X, label=f"N={N}")

    plt.xlabel("range [m]")
    plt.ylabel("Max amplitude")
    plt.title("Max amplitude vs Range at Different N's")
    plt.legend()
    plt.yscale("log")
    plt.grid(which="both")
    plt.show()


