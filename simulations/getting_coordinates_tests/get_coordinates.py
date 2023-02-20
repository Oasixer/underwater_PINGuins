import numpy as np
import random
from math import acos, cos, sin, sqrt, pow, pi


def dist(p1, p2):
    return sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)


if __name__ == "__main__":
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

                        dists = [[dist(p1, p2) for p2 in actual] for p1 in actual]

                        angles = [0, 0, 0, 0]

                        angles[2] = acos(
                            (dists[0][1]**2 + dists[0][2]**2 - dists[1][2]**2)
                            / (2 * dists[0][1] * dists[0][2]))

                        guess_using_1 = [
                            acos((dists[0][1] ** 2 + dists[0][3] ** 2 - dists[1][3] ** 2) / (2 * dists[0][1] * dists[0][3])),
                            -acos((dists[0][1] ** 2 + dists[0][3] ** 2 - dists[1][3] ** 2) / (2 * dists[0][1] * dists[0][3]))]

                        triangle = (dists[0][2] ** 2 + dists[0][3] ** 2 - dists[2][3] ** 2) / (2 * dists[0][2] * dists[0][3])
                        triangle = min(max(triangle, -1.0), 1.0)
                        angle_2_to_3 = acos(triangle)
                        guess_using_2 = [
                            angle_2_to_3 + angles[2],
                            -angle_2_to_3 + angles[2],
                        ]
                        for i in range(2):
                            if guess_using_2[i] > pi:
                                guess_using_2[i] = guess_using_2[i] - 2*pi

                        differences = [
                            abs(guess_using_1[0] - guess_using_2[0]),
                            abs(guess_using_1[0] - guess_using_2[1]),
                            abs(guess_using_1[1] - guess_using_2[0]),
                            abs(guess_using_1[1] - guess_using_2[1]),
                        ]
                        values = [
                            (guess_using_1[0] + guess_using_2[0]) / 2,
                            (guess_using_1[0] + guess_using_2[1]) / 2,
                            (guess_using_1[1] + guess_using_2[0]) / 2,
                            (guess_using_1[1] + guess_using_2[1]) / 2,
                        ]

                        angles[3] = values[np.argmin(differences)]

                        coords = []
                        for d, ang in zip(dists[0], angles):
                            coords.append([d * cos(ang), d * sin(ang)])
                        diff = np.max(np.abs(np.array(coords) - np.array(actual)))
                        if diff > 1e-5:
                            print("actual: ", actual, "coords:", coords)
    print("done")
