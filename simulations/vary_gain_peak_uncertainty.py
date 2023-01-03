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
MAX_VOLTAGE = 3.3  # V

# Experiment Constants
N_EXPERIMENTS = 1000


if __name__ == "__main__":
    ranges = np.arange(1, 50, 0.05)  # m
    e = np.tile(np.exp(-2j*pi * K * np.arange(N) / N), [ranges.shape[0], 1])

    gains = [20, 50, 100, 200, 500, 1000]  # V/V

    # Generate data at different ranges
    t = np.arange(0, 3*N/F_SAMPLE, 1/F_SAMPLE)
    received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
    received = np.zeros(t.shape)
    received[N: 2*N] = np.sin(2*pi * F_SENT * t[N: 2*N])
    received = np.tile(received, [ranges.shape[0], 1]) * np.expand_dims(received_voltage, -1)

    all_i_peaks = []
    for gain in gains:
        gain_i_peaks = []
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

            gain_i_peaks.append(np.argmax(X, axis=-1))
        all_i_peaks.append(gain_i_peaks)

    t_diff = np.abs((np.array(all_i_peaks) - 2*N + 1) / F_SAMPLE)

    mean_d_diffs = np.mean(t_diff, axis=1) * 1500
    max_d_diffs = np.max(t_diff, axis=1) * 1500

    np.save("Mean dist uncertainty With Different gains", mean_d_diffs)
    np.save("Max dist uncertainty With Different gains", max_d_diffs)

    for mean_d_diff in mean_d_diffs:
        plt.plot(ranges, mean_d_diff)
    plt.xlabel("Range [m]")
    plt.ylabel("Mean Dist Diff [m]")
    plt.yscale("log")
    plt.title("Mean dist uncertainty With Different gains")
    plt.grid(which="both")
    plt.legend(gains, title="gain")
    plt.show()

    for max_d_diff in max_d_diffs:
        plt.plot(ranges, max_d_diff)
    plt.xlabel("Range [m]")
    plt.ylabel("Max Dist Diff [m]")
    plt.yscale("log")
    plt.title("Max dist uncertainty With Different gains")
    plt.grid(which="both")
    plt.legend(gains, title="gain")
    plt.show()
