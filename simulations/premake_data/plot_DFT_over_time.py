import matplotlib.pyplot as plt
import numpy as np
from math import pi, sin, cos

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
CIRCUIT_GAIN = 100  # V / V
MAX_VOLTAGE = 3.3  # V

# Experiment Constants
T_TOTAL = 3.3  # s
T_ACTIVE_START = 1.0  # s
I_ACTIVE_START = int(F_SAMPLE / T_ACTIVE_START)
N_EXPERIMENTS = 50

if __name__ == "__main__":
    dist = 30  # m
    i_dist = int((dist - 1) * 2)

    t = np.arange(0, T_TOTAL, 1/F_SAMPLE)
    t = t[I_ACTIVE_START - 1*N: I_ACTIVE_START + 3*N]

    for ex in range(10):
        data = np.load(f"data/X_{ex}.npy")
        data = data[:, I_ACTIVE_START - 1*N: I_ACTIVE_START + 3*N]
        plt.figure()
        plt.plot(t, data[i_dist])
        plt.xlabel("Time [s]")
        plt.ylabel("|X(freq)|")
        plt.title(f"Amplitude over time at {dist} m")
        plt.show()
