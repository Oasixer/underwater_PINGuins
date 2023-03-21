import numpy as np
import os
from scipy.io import savemat
import re


if __name__ == "__main__":
    dictionary = {
        2: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-15-08",
        4: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-17-15",
        6: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-18-35",
        8: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-20-18",
        10: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-21-17",
        12: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-23-20",
        14: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-24-32",
        16: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-25-41",
        18: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-27-21",
        20: r"../data_stream/04e9e5143f1e/cli_data/2023-03-13_13-29-05",
    }
    valid_readings = []

    for real_dist, path in dictionary.items():

        file = open(path, 'r')
        segments = file.read().split("###")

        for segment in segments[1:]:

            section = segment[segment.find("["):]
            dists = section[section.find("[")+1: section.find("]")].replace("\n", "").replace("<", "").split("; ")[:-1]
            dists = [float(x) for x in dists]

            section = section[section[1:].find("["):]
            trip_times = section[section.find("[")+1: section.find("]")].replace("\n", "").replace("<", "").split("; ")[:-1]
            trip_times = [int(x) for x in trip_times]

            section = section[section[3:].find("["):]
            magnitudes = section[section.find("[")+1: section.find("]")].replace("\n", "").replace("<", "").split("; ")[:-1]
            magnitudes = [int(x) for x in magnitudes]

            for i in range(len(trip_times)):
                if dists[i] > 0 and trip_times[i] < 300_000:
                    valid_readings.append([real_dist, dists[i], trip_times[i], magnitudes[i]])

    savemat("data.mat", {"data": np.array(valid_readings)})
