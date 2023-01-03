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
    all_t_means = []
    all_t_maxs = []
    for N in Ns:
        # DFT variables
        K = F_SENT * N / F_SAMPLE
        EXP = np.exp(2j * pi * K / N)

        t = np.arange(0, 3*N/F_SAMPLE, 1/F_SAMPLE)
        received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
        received = np.zeros([ranges.shape[0], t.shape[0]])
        received[:, N: 2*N] = np.tile(np.sin(2 * pi * F_SENT * t[N: 2*N]), [980, 1]) * np.expand_dims(received_voltage, -1)

        e = np.tile(np.exp(-2j*pi * K * np.arange(N) / N), [980, 1])

        i_peaks = []
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

            i_peaks.append(np.argmax(X, axis=-1))

        t_diff = (np.array(i_peaks) - 2*N + 1) / F_SAMPLE
        t_diff = np.abs(t_diff)

        mean_t_diff = np.mean(t_diff, axis=0)
        max_t_diff = np.max(t_diff, axis=0)

        all_t_means.append(mean_t_diff)
        all_t_maxs.append(max_t_diff)

    all_d_means = np.array(all_t_means) * 1500
    all_d_maxs = np.array(all_t_maxs) * 1500

    np.save("mean dist uncertainty with different Ns", all_d_means)
    np.save("max dist uncertainty with different Ns", all_d_maxs)

    for mean_d_diff in all_d_means:
        plt.plot(ranges, mean_d_diff)
    plt.xlabel("Range [m]")
    plt.ylabel("Mean Dist Diff [m]")
    plt.yscale("log")
    plt.title("Mean dist uncertainty with different N's")
    plt.grid(which="both")
    plt.legend(Ns)
    plt.show()

    for max_d_diff in all_d_maxs:
        plt.plot(ranges, max_d_diff)
    plt.xlabel("Range [m]")
    plt.ylabel("Max Dist Diff [m]")
    plt.title("Mean dist uncertainty with different N's")
    plt.yscale("log")
    plt.grid(which="both")
    plt.legend(Ns, title="N")
    plt.show()
