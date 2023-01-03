import numpy as np
from numpy import pi

import constants


def bits_to_amplitude(bits: np.ndarray) -> (np.ndarray, np.ndarray):
    if bits.shape[0] != constants.SZ_MSG:
        print("incorrect msg size given. Exiting")
        raise "Incorrect msg size"
    if bits.dtype != np.bool:
        print("incorrect dtype of bits given")
        raise "Incorrect dtype"

    t_extra_beginning = 0.005

    t = np.arange(0, constants.T_MSG + t_extra_beginning, constants.T_SAMPLE)
    amplitude = np.zeros(int((constants.T_MSG + t_extra_beginning) * constants.F_SAMPLE))

    for i, bit in enumerate(bits):
        freq = constants.F_HIGH if bit else constants.F_LOW

        active_start = int(i * constants.T_BIT * constants.F_SAMPLE + t_extra_beginning * constants.F_SAMPLE)
        active_end = int(active_start + constants.T_ACTIVE * constants.F_SAMPLE)

        amplitude[active_start: active_end] = np.sin(2*pi*freq * t[active_start: active_end])

    return t, amplitude


def demo_amplitude_generator(freqs: list[list[int]]) -> (np.ndarray, np.ndarray):
    t_extra_beginning = 0.0025
    t_total = len(freqs) * constants.T_BIT + t_extra_beginning

    t = np.arange(0, t_total, constants.T_SAMPLE)
    amplitude = np.zeros(int(t_total * constants.F_SAMPLE))

    for i, msg in enumerate(freqs):
        active_start = int(i * constants.T_BIT * constants.F_SAMPLE + t_extra_beginning * constants.F_SAMPLE)
        active_end = int(active_start + constants.T_ACTIVE * constants.F_SAMPLE)

        for freq in msg:
            amplitude[active_start: active_end] += 0.2 * np.sin(2*pi*freq * t[active_start: active_end])

    return t, amplitude


def add_noise(data: np.ndarray) -> np.ndarray:
    data += np.random.normal(0, 1, size=data.shape)

    return data


def make_integers(data: np.array, n_bits: int) -> np.ndarray:
    print(f"start min: {np.min(data)}, start max: {np.max(data)}")
    data -= np.min(data)
    data /= np.max(data)
    data *= (2 ** n_bits - 1)
    data = np.floor(data)
    data -= (2 ** (n_bits-1))
    print(f"end min: {np.min(data)}, end max: {np.max(data)}")
    return data
