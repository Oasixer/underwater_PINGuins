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
CIRCUIT_GAIN = 100  # V / V
MAX_VOLTAGE = 3.3  # V

# Experiment Constants
T_TOTAL = 3.3  # s
T_ACTIVE_START = 1.0  # s
I_ACTIVE_START = int(F_SAMPLE / T_ACTIVE_START)
N_EXPERIMENTS = 50

if __name__ == "__main__":
    data = np.load(f"data/X_{0}.npy")
    data = np.concatenate([data[:, :I_ACTIVE_START], data[:, I_ACTIVE_START + 2*N:]], axis=-1)

    max_amp = np.max(data)
    min_amp = np.min(data)

    counts, bins = np.histogram(data, 1000)

    print(f"max: {max_amp} , min: {min_amp}")

    plt.stairs(counts, bins)
    plt.xlabel("Amplitude")
    plt.ylabel("Count")
    plt.title("Noise characterization with gain 100")
    plt.show()
