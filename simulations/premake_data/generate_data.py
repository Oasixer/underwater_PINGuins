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
N_EXPERIMENTS = 100

if __name__ == "__main__":
    ranges = np.arange(1, 50, 0.5)  # m
    for i_experiment in range(N_EXPERIMENTS):
        print(f"{i_experiment}/{N_EXPERIMENTS}")

        # Create data
        t = np.arange(0, T_TOTAL, 1 / F_SAMPLE)
        received = np.zeros(t.shape)
        received[I_ACTIVE_START: I_ACTIVE_START + N] = np.sin(2 * pi * F_SENT * t[I_ACTIVE_START: I_ACTIVE_START + N])

        # Receive at different ranges
        received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
        received = np.tile(received, [ranges.shape[0], 1]) * np.expand_dims(received_voltage, -1)

        # Go through amplifier circuit
        received += np.random.normal(0, ESTIMATED_NOISE, received.shape)  # add Gaussian noise
        received *= CIRCUIT_GAIN  # add gain
        received += MAX_VOLTAGE/2  # add offset
        received = np.maximum(np.minimum(received, MAX_VOLTAGE), 0)  # hit the rails

        # Go through ADC
        received = np.round(received / MAX_VOLTAGE * 4096)  # scale and discretize

        # Normalize before Fourier Transform
        received = (received - 2048) / 4096
        received = received.astype(complex)

        # Fourier Transform
        X = np.zeros(received.shape).astype(complex)
        for n in range(t.shape[0]):  # Sliding DFT
            X[:, n] = (X[:, n-1] - received[:, n-N] + received[:, n]) * EXP

        X = np.abs(X) / (N/2)

        np.save(f"data/X_{i_experiment}", X)
    print("Done Generating Data")
