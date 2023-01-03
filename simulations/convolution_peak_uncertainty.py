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
N_EXPERIMENTS = 100


if __name__ == "__main__":
    ranges = np.arange(1, 50, 0.5)  # m

    # Initialize vector to convolve with to get fourier transform
    e = np.exp(-2j * pi * K * np.arange(N) / N)

    # initialize triangle kernel to convolve with to get peak
    triangle_kernel = np.zeros(2 * N)
    triangle_kernel[:N] = np.arange(N) / N
    triangle_kernel[N:] = (N - np.arange(N)) / N

    # initialize longer kernel to convolve with to get peak
    longer_kernel = np.zeros(3 * N)
    longer_kernel[N: 2*N] = np.arange(N) / N
    longer_kernel[2*N:] = (N - np.arange(N)) / N

    # Create data
    t = np.arange(0, 6*N/F_SAMPLE, 1/F_SAMPLE)
    x = np.zeros(t.shape)
    x[2*N: 3*N] = np.sin(2 * pi * F_SENT * t[2*N: 3*N])

    # Receive at different ranges
    received_voltage = P_SENT / pow(ranges, 2) * RECEIVE_GAIN / 2  # V
    x = np.tile(x, [ranges.shape[0], 1]) * np.expand_dims(received_voltage, -1)

    i_peaks_naive = []
    i_peaks_triangle = []
    i_peaks_longer = []
    for i_experiment in range(N_EXPERIMENTS):
        print(i_experiment)
        # Go through amplifier circuit
        x += np.random.normal(0, ESTIMATED_NOISE, x.shape)  # add Gaussian noise
        x *= CIRCUIT_GAIN  # add gain
        x += MAX_VOLTAGE / 2  # add offset
        x = np.maximum(np.minimum(x, MAX_VOLTAGE), 0)  # hit the rails

        # Go through ADC
        x = np.round(x / MAX_VOLTAGE * 4096)  # scale and discretize

        # Normalize before Fourier Transform
        x = (x - 2048) / 4096
        x = x.astype(complex)

        # Fourier Transform
        Xs = np.apply_along_axis(lambda m: np.convolve(m, e, mode='full'), axis=-1, arr=x)[:, :x.shape[1]]
        Xs = np.abs(Xs) / (N / 2)

        # peak using naive approach
        i_peaks_naive.append(np.argmax(Xs, axis=-1))

        # convolve with triangle kernel
        triangle_convolved = np.apply_along_axis(lambda m: np.convolve(m, triangle_kernel, mode='full'), axis=-1, arr=Xs)[:, :Xs.shape[1]]
        i_peaks_triangle.append(np.argmax(triangle_convolved, axis=-1))

        # convolve with triangle kernel
        longer_convolved = np.apply_along_axis(lambda m: np.convolve(m, longer_kernel, mode='full'), axis=-1, arr=Xs)[:, :Xs.shape[1]]
        i_peaks_longer.append(np.argmax(longer_convolved, axis=-1))

        # for dist in [1, 10, 20, 25, 30, 49.5]:
        #     i_dist = int((dist - 1) * 2)
        #     # plt.plot(t, x[i_dist], label="x")
        #     plt.plot(t, Xs[i_dist], label="X")
        #     plt.plot(t, triangle_convolved[i_dist]/625, label="triangle")
        #     plt.plot(t, longer_convolved[i_dist]/625, label="longer")
        #     plt.title(f"at {dist} m")
        #     plt.legend()
        #     plt.grid(which="both")
        #     plt.show()

    t_peaks_naive = np.abs((np.array(i_peaks_naive) - 3*N + 1) / F_SAMPLE)
    t_peaks_triangle = np.abs((np.array(i_peaks_triangle) - 4*N + 1) / F_SAMPLE)
    t_peaks_longer = np.abs((np.array(i_peaks_longer) - 5*N + 1) / F_SAMPLE)

    samples = np.random.choice([row for row in t_peaks_naive], (100, 10))

    mean_d_naive = np.mean(t_peaks_naive, axis=0) * 1500
    mean_d_triangle = np.mean(t_peaks_triangle, axis=0) * 1500
    mean_d_longer = np.mean(t_peaks_longer, axis=0) * 1500

    max_d_naive = np.max(t_peaks_naive, axis=0) * 1500
    max_d_triangle = np.max(t_peaks_triangle, axis=0) * 1500
    max_d_longer = np.max(t_peaks_longer, axis=0) * 1500

    plt.plot(ranges, mean_d_naive, label="naive")
    plt.plot(ranges, mean_d_triangle, label="triangle")
    # plt.plot(ranges, mean_d_longer, label="longer")
    plt.xlabel("Range [m]")
    plt.ylabel("Mean Dist Diff [m]")
    plt.title("Mean dist uncertainty over range")
    plt.grid(which="both")
    plt.legend()
    plt.yscale("log")
    plt.show()

    plt.plot(ranges, max_d_naive, label="naive")
    plt.plot(ranges, max_d_triangle, label="triangle")
    # plt.plot(ranges, max_d_longer, label="longer")
    plt.xlabel("Range [m]")
    plt.ylabel("Max Dist Diff [m]")
    plt.title("Max dist uncertainty over range")
    plt.grid(which="both")
    plt.legend()
    plt.yscale("log")
    plt.show()
