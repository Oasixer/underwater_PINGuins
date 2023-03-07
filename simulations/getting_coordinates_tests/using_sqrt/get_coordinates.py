import numpy as np
import random
from math import acos, cos, sin, sqrt, pow, pi


def dist(p1, p2):
    return sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)


def valid_sqrt(val):
    if val > 0:
        return sqrt(val)
    else:
        return 0


def calibrate(dists):
    coords = np.zeros((4, 2))

    coords[0] = [0, 0]
    coords[1] = [dists[0][1], 0]

    x2 = (dists[0][2]**2 + dists[0][1]**2 - dists[1][2]**2) / (2 * dists[0][1])
    y2 = valid_sqrt(dists[0][2]**2 - x2**2)
    coords[2] = [x2, y2]

    x3 = (dists[0][1]**2 + dists[0][3]**2 - dists[1][3]**2) / (2 * dists[0][1])
    possible_y3s = [
        valid_sqrt(dists[0][3]**2 - x3**2),
        -valid_sqrt(dists[0][3]**2 - x3**2),
        y2 + valid_sqrt(dists[2][3]**2 - (x2 - x3)**2),
        y2 - valid_sqrt(dists[2][3]**2 - (x2 - x3)**2),
    ]
    differences_between_y3s = [
        abs(possible_y3s[0] - possible_y3s[2]),
        abs(possible_y3s[0] - possible_y3s[3]),
        abs(possible_y3s[1] - possible_y3s[2]),
        abs(possible_y3s[1] - possible_y3s[3]),
    ]
    means_of_y3s = [
        (possible_y3s[0] + possible_y3s[2]) / 2,
        (possible_y3s[0] + possible_y3s[3]) / 2,
        (possible_y3s[1] + possible_y3s[2]) / 2,
        (possible_y3s[1] + possible_y3s[3]) / 2,
    ]
    y3 = means_of_y3s[np.argmin(differences_between_y3s)]
    coords[3] = [x3, y3]

    return coords


def test_all_quadrants_randomly():
    for x_1 in range(1, 10):
        for x_2 in range(-10, 10):
            for y_2 in range(1, 10):
                for x_3 in range(-10, 10):
                    for y_3 in range(-10, 10):
                        if x_3 == 0 and y_3 == 0:
                            continue
                        actual = [
                            [0, 0],
                            [x_1, 0],
                            [x_2, y_2],
                            [x_3, y_3]
                        ]
                        dists_actual = np.array([[dist(p1, p2) for p2 in actual] for p1 in actual])

                        for _ in range(1000):
                            rand_nums = np.random.randn(6) * 0.1
                            dists = np.array([
                                [dists_actual[0][0],
                                 dists_actual[0][1] + rand_nums[0],
                                 dists_actual[0][2] + rand_nums[1],
                                 dists_actual[0][3] + rand_nums[2]],
                                [dists_actual[1][0] + rand_nums[0],
                                 dists_actual[1][1],
                                 dists_actual[1][2] + rand_nums[3],
                                 dists_actual[1][3] + rand_nums[4]],
                                [dists_actual[2][0] + rand_nums[1],
                                 dists_actual[2][1] + rand_nums[3],
                                 dists_actual[2][2],
                                 dists_actual[2][3] + rand_nums[5]],
                                [dists_actual[3][0] + rand_nums[2],
                                 dists_actual[3][1] + rand_nums[4],
                                 dists_actual[3][2] + rand_nums[5],
                                 dists_actual[3][3]],
                            ])
                            dists = np.maximum(dists, 0)

                            coords = calibrate(dists)

                            diff = np.max(np.abs(np.array(coords) - np.array(actual)))
                            if diff > 1:
                                coords = np.round(coords, 2).tolist()
                                print("actual: ", actual, "coords:", coords, "rand: ", rand_nums)
    print("done")


def test_all_quadrants():
    for x_1 in range(1, 10):
        for x_2 in range(-10, 10):
            for y_2 in range(1, 10):
                for x_3 in range(-10, 10):
                    for y_3 in range(-10, 10):
                        if x_3 == 0 and y_3 == 0:
                            continue
                        actual = [
                            [0, 0],
                            [x_1, 0],
                            [x_2, y_2],
                            [x_3, y_3]
                        ]
                        dists = np.array([[dist(p1, p2) for p2 in actual] for p1 in actual])

                        coords = calibrate(dists)

                        diff = np.max(np.abs(np.array(coords) - np.array(actual)))
                        if diff > 1e-5:
                            coords = np.round(coords, 2).tolist()
                            print("actual: ", actual, "coords:", coords)
    print("done")


def test_random(actual, std_dev):
    dists_actual = np.array([[dist(p1, p2) for p2 in actual] for p1 in actual])

    errors = []

    for _ in range(10000):
        rand_nums = np.random.randn(6) * std_dev
        dists = np.array([
            [dists_actual[0][0],
             dists_actual[0][1] + rand_nums[0],
             dists_actual[0][2] + rand_nums[1],
             dists_actual[0][3] + rand_nums[2]],
            [dists_actual[1][0] + rand_nums[0],
             dists_actual[1][1],
             dists_actual[1][2] + rand_nums[3],
             dists_actual[1][3] + rand_nums[4]],
            [dists_actual[2][0] + rand_nums[1],
             dists_actual[2][1] + rand_nums[3],
             dists_actual[2][2],
             dists_actual[2][3] + rand_nums[5]],
            [dists_actual[3][0] + rand_nums[2],
             dists_actual[3][1] + rand_nums[4],
             dists_actual[3][2] + rand_nums[5],
             dists_actual[3][3]],
        ])
        dists = np.maximum(dists, 0.001)

        coords = calibrate(dists)

        diff = np.array(coords) - np.array(actual)
        errors.append(diff.reshape(-1))
        # if np.max(np.abs(diff)) > 1:
        #     coords = np.round(coords, 2).tolist()
        #     print("actual: ", actual, "coords:", coords, "rand: ", rand_nums)
    print("done")
    errors = np.array(errors)
    print("Error: mean: ", np.mean(errors), ", std dev: ", np.std(errors))


if __name__ == "__main__":
    actual = [
        [0, 0],
        [3, 0],
        [-1, 2],
        [-1, -2]
    ]
    std_dev = 0.1  # m
    test_random(actual, std_dev)
