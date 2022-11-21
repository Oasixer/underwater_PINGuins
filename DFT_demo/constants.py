F_SAMPLE = 500_000  # Hz
DUTY_CYCLE = 0.40
SZ_MSG = 42  # bits
N_STATIONARY = 3  # number of helper nodes
SZ_WINDOW = 1250  # samples
T_MSG = 0.42  # seconds

F_LOW = 10_000  # Hz
F_HIGH = 18_000  # Hz
F_ALL = 26_000  # Hz
F_NODES = [34_000, 42_000, 50_000]  # Hz

F_BITRATE = SZ_MSG / T_MSG  # bits / second
T_BIT = 1 / F_BITRATE  # seconds
T_ACTIVE = T_BIT * DUTY_CYCLE  # seconds
T_INACTIVE = T_BIT - T_ACTIVE  # seconds

T_WINDOW = SZ_WINDOW / F_SAMPLE  # seconds
T_SAMPLE = 1 / F_SAMPLE  # seconds

SZ_FREQ_DOMAIN = SZ_WINDOW // 2
