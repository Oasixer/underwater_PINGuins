import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from collections import deque


F_SAMPLE = 500_000 # a2d sampling frequency
F_SAMPLE_mHZ = F_SAMPLE / 1000 # a2d sampling frequency

T_SAMPLE = 1/F_SAMPLE_mHz  # a2d sample period, ms

N_SLAVES = 4 # number of slave nodes

#  F_NEUTRAL = 20_000 # FM neutral frequency, hz
#  F_LOW = 15_000 # FM neutral frequency, hz
#  F_HIGH = 25_000 # FM neutral frequency, hz

F_NEUTRAL = 20_000 # FM neutral frequency, hz
F_LOW = 15_000 # FM neutral frequency, hz
F_HIGH = 25_000 # FM neutral frequency, hz

SZ_MSG = 80 # bits per msg
T_MSG_S = 1 # msg tx time, s
DUTY_CYCLE = 0.25 # to avoid reflection, send bit then shut up for (1-DUTY_CYCLE)*T_MSG
F_BITRATE = SZ_MSG / T_MSG # bitrate within message, bit/s
T_BIT = 1 / F_BITRATE * 1000 # bit period, ms

T_ACTIVE = DUTY_CYCLE * T_BIT # active period during tx of bit
T_INACTIVE = (1 - DUTY_CYCLE) * T_BIT

T_FFT_BIT = 0.01 # !!!!!!!!!!!!!!!!!!! should be milliseconds instead, also should be calculated??

T_WINDOW = T_BIT * T_FFT_BIT # duration of DFT window which will slide along signal, ms

class Bit(Enum):
    LOW = F_LOW
    NEUTRAL = F_NEUTRAL
    HIGH = F_HIGH

def int_to_bits(i: int) -> list[bit]:
    def _bit_str_to_bit(bit_str: str) -> bit:
        if bit_str == '0':
            return Bit.LOW
        if bit_str == '1':
            return Bit.HIGH
        return Bit.NEUTRAL
    return [_bit_str_to_bit(j) for j in bin(i)]

msg = int_to_bits(69)

timestamps = deque([0])
amplitudes = deque([])
for bit in msg:
    ts_begin_active = timestamps[-1] # timestamp when bit started tx, ms
    ts_end_active = ts_begin_active + T_ACTIVE # timestamp when finished sending bit, ms
    ts_end_inactive = ts_begin_inactive + T_INACTIVE # timestamp when finished sending bit, ms

    f_bit_mHz = bit.value / 1000

    # active period
    while((ts_cur := timestamps[-1]) < ts_end_active):
        sample = np.cos(2*pi*f_bit_mHz*ts_cur)
        amplitudes.append(sample)
        timestamps.append(ts_cur + T_SAMPLE)

    # inactive period
    while((ts_cur := timestamps[-1]) < ts_end_inactive):
        sample = 0
        amplitudes.append(sample)
        timestamps.append(ts_cur + T_SAMPLE)

timestamps.pop()
