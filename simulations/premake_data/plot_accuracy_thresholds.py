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
N_EXPERIMENTS = 100
N_CONSECUTIVE = 3

if __name__ == "__main__":
    ranges = np.arange(1, 50, 0.5)  # m
    thresholds = [0.004, 0.005, 0.006]

    all_true_positives = []
    all_false_positives = []
    for i in range(N_EXPERIMENTS):
        data = np.load(f"data/X_{i}.npy")

        sample_true_positives = []
        sample_false_positives = []
        for threshold in thresholds:
            is_larger = data >= threshold
            is_larger_consecutive = []
            for row in is_larger:
                is_larger_consecutive.append(np.convolve(row, np.ones(N_CONSECUTIVE, dtype=int), 'valid') >= N_CONSECUTIVE)
            is_larger_consecutive = np.array(is_larger_consecutive)
            positives = is_larger_consecutive.sum(axis=-1)
            true_positives = (is_larger_consecutive[:, I_ACTIVE_START: I_ACTIVE_START + 2*N]).sum(axis=-1)
            false_positives = positives - true_positives
            sample_true_positives.append(true_positives)
            sample_false_positives.append(false_positives)

        all_true_positives.append(sample_true_positives)
        all_false_positives.append(sample_false_positives)

    all_true_positives = np.array(all_true_positives)
    all_false_positives = np.array(all_false_positives)

    all_false_negatives = all_true_positives.shape[0] - (all_true_positives > 0).sum(axis=0)
    all_false_positives = all_false_positives.sum(axis=0)

    plt.figure()
    for i in range(len(thresholds)):
        plt.plot(ranges, all_false_positives[i, :])
    plt.xlabel("range [m]")
    plt.ylabel("Total Count False Positives")
    plt.title("False Positives at different thresholds")
    plt.legend([f"{threshold}" for threshold in thresholds])
    plt.yscale("log")
    plt.grid(which="both")
    plt.show()

    plt.figure()
    for i in range(len(thresholds)):
        plt.plot(ranges, all_false_negatives[i, :])
    plt.xlabel("range [m]")
    plt.ylabel("Total Count Missed Detections")
    plt.title("Number of Missed Detections at different thresholds")
    plt.legend([f"{threshold}" for threshold in thresholds])
    plt.grid(which="both")
    plt.show()
