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
    thresholds = [0.0086, 0.0066, 0.006, 0.0035, 0.0024]

    all_detected = []
    all_false_alarms = []
    for N, threshold in zip(Ns, thresholds):
        # DFT variables
        K = F_SENT * N / F_SAMPLE
        EXP = np.exp(2j * pi * K / N)

        t = np.arange(0, 4*N/F_SAMPLE, 1/F_SAMPLE)
        received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
        received = np.zeros([ranges.shape[0], t.shape[0]])
        received[:, N: 2*N] = np.tile(np.sin(2 * pi * F_SENT * t[N: 2*N]), [980, 1]) * np.expand_dims(received_voltage, -1)

        e = np.tile(np.exp(-2j*pi * K * np.arange(N) / N), [980, 1])

        experiments_detected = []
        experiments_false_alarms = []
        for i in range(N_EXPERIMENTS):
            print(f"N: {N}, i: {i}")
            data = received + np.random.normal(0, ESTIMATED_NOISE, received.shape)  # add Gaussian noise
            data *= CIRCUIT_GAIN  # add gain
            data += MAX_VOLTAGE / 2  # add offset
            data = np.maximum(np.minimum(data, MAX_VOLTAGE), 0)  # hit the rails

            # Go through ADC
            data = np.round(data / MAX_VOLTAGE * 4096)  # scale and discretize

            # Normalize before Fourier Transform
            data = (data - 2048) / 4096
            data = data.astype(complex)

            # Fourier Transform
            e = np.exp(-2j * pi * K * np.arange(N) / N)
            X = np.apply_along_axis(lambda m: np.convolve(m, e, mode='full'), axis=-1, arr=data)[:, :data.shape[1]]
            X = np.abs(X) / (N / 2)

            is_larger = (X >= threshold)

            active_period = is_larger[:, N: 3*N]
            detected = np.sum(active_period, axis=-1) > 0
            experiments_detected.append(detected)

            inactive_period = np.concatenate((is_larger[:, :N], is_larger[:, 3*N:]), axis=-1)
            false_alarms = np.sum(inactive_period, axis=-1) > 0
            experiments_false_alarms.append(false_alarms)
        all_detected.append(experiments_detected)
        all_false_alarms.append(experiments_false_alarms)

    counts_detected = np.sum(np.array(all_detected), axis=1)
    counts_false_alarms = np.sum(np.array(all_false_alarms), axis=1)

    for N, n_detected in zip(Ns, counts_detected):
        plt.plot(ranges, N_EXPERIMENTS - n_detected, label=N)
    plt.ylabel("Count Missed")
    plt.xlabel("Range [m]")
    plt.title("Missed Detections with different N's")
    plt.grid(which="both")
    plt.legend(title="N")
    plt.savefig("Missed Detections with different Ns")
    plt.show()

    for N, n_false_alarms in zip(Ns, counts_false_alarms):
        plt.plot(ranges, n_false_alarms, label=N)
    plt.ylabel("Count Missed")
    plt.xlabel("Range [m]")
    plt.title("False Alarms with different N's")
    plt.grid(which="both")
    plt.legend(title="N")
    plt.savefig("False Alarms with different Ns")
    plt.show()
