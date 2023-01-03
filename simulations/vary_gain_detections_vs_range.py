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
N_EXPERIMENTS = 1000

if __name__ == "__main__":
    ranges = np.arange(1, 50, 0.5)  # m
    gains = [20, 50, 100, 200, 500, 1000]  # V/V
    thresholds = [6e-5 * gain for gain in gains]

    e = np.tile(np.exp(-2j*pi * K * np.arange(N) / N), [ranges.shape[0], 1])

    # Generate data at different ranges
    t = np.arange(0, 4*N/F_SAMPLE, 1/F_SAMPLE)
    received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
    received = np.zeros(t.shape)
    received[N: 2*N] = np.sin(2*pi * F_SENT * t[N: 2*N])
    received = np.tile(received, [ranges.shape[0], 1]) * np.expand_dims(received_voltage, -1)

    all_detected = []
    all_false_alarms = []
    for gain, threshold in zip(gains, thresholds):
        gain_detected = []
        gain_false_alarms = []
        for i_experiment in range(N_EXPERIMENTS):
            print(f"gain: {gain}, i: {i_experiment}")
            # Go through amplifier circuit
            data = received + np.random.normal(0, ESTIMATED_NOISE, received.shape)  # add Gaussian noise
            data *= gain  # add gain
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
            gain_detected.append(detected)

            inactive_period = np.concatenate((is_larger[:, :N], is_larger[:, 3*N:]), axis=-1)
            false_alarms = np.sum(inactive_period, axis=-1) > 0
            gain_false_alarms.append(false_alarms)
        all_detected.append(gain_detected)
        all_false_alarms.append(gain_false_alarms)

    counts_detected = np.sum(np.array(all_detected), axis=1)
    counts_false_alarms = np.sum(np.array(all_false_alarms), axis=1)

    for gain, n_detected in zip(gains, counts_detected):
        plt.plot(ranges, N_EXPERIMENTS - n_detected, label=gain)
    plt.ylabel("Count Missed")
    plt.xlabel("Range [m]")
    plt.title("Missed Detections with different gain")
    plt.grid(which="both")
    plt.legend(title="gain")
    plt.savefig("Missed Detections with different gain")
    plt.show()

    for gain, n_false_alarms in zip(gains, counts_false_alarms):
        plt.plot(ranges, n_false_alarms, label=gain)
    plt.ylabel("Count Missed")
    plt.xlabel("Range [m]")
    plt.title("False Alarms with different gain")
    plt.grid(which="both")
    plt.legend(title="gain")
    plt.savefig("False Alarms with different gain")
    plt.show()
