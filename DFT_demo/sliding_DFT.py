import numpy as np
from numpy import pi

import constants


class SlidingDFT:
    def __init__(self) -> None:
        self.time_domain_values = [0.0] * constants.SZ_WINDOW
        self.freq_domain = np.zeros(constants.SZ_FREQ_DOMAIN)

        self.natural_frequencies = np.exp(2j * pi * np.arange(constants.SZ_FREQ_DOMAIN) / constants.SZ_WINDOW)

    def update(self, new_sample: float) -> None:
        # update the frequency domain
        self.freq_domain = (self.freq_domain - self.time_domain_values[0] + new_sample) * self.natural_frequencies

        # update the time domain
        self.time_domain_values.pop(0)
        self.time_domain_values.append(new_sample)
