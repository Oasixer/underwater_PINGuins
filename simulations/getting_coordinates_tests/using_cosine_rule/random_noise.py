import numpy as np
import random
from math import acos, cos, sin, sqrt, pow, pi


def dist(p1, p2):
    return sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)

def cosine_rule(adj1, adj2, opp):
    cos_angle = (adj1**2 + adj2**2 - opp**2) / (2 * adj1 * adj2)
    if cos_angle > 1.0:
        cos_angle = 1.0
    elif cos_angle < -1.0:
        cos_angle = -1.0
    return acos(cos_angle)


if __name__ == "__main__":
    variance = 0.2

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

                            angles = [0, 0, 0, 0]

                            angles[2] = cosine_rule(dists[0][1], dists[0][2], dists[1][2])

                            guess_using_1 = [
                                cosine_rule(dists[0][1], dists[0][3], dists[1][3]),
                                -cosine_rule(dists[0][1], dists[0][3], dists[1][3])]

                            angle_2_to_3 = cosine_rule(dists[0][2], dists[0][3], dists[2][3])
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
                            if diff > 10:
                                coords = np.round(coords, 2).tolist()
                                print("actual: ", actual, "coords:", coords, "theta_b: ", angles[2], "rand: ", rand_nums)
    print("done")
