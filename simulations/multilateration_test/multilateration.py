import numpy as np
from math import fabs, cos, sin, sqrt, atan2
from matplotlib import pyplot as plt

class Coord3d:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z


class Coord2d:
    def __init__(self, x, y):
        self.x = x
        self.y = y


def convert_dist_to_2d(dist_3d, depth1, depth2):
    if dist_3d > fabs(depth1 - depth2):
        return sqrt(dist_3d**2 - (depth1 - depth2)**2)
    return 0.0


def get_points_of_intersection(coord1, coord2, dist1, dist2):
    diff = sqrt((coord1.x - coord2.x)**2 + (coord1.y - coord2.y)**2)
    angle = atan2(coord2.y - coord1.y, coord2.x - coord1.x)

    x_local = (dist1**2 - dist2**2 + diff**2) / (2 * diff)
    try:
        if dist1 > fabs(x_local):
            y1_local = sqrt(dist1**2 - x_local**2)
        else:
            y1_local = 0.0
    except ValueError:
        print("wtf")
    y2_local = -y1_local

    return [
        Coord2d(
            x_local*cos(angle) - y1_local*sin(angle) + coord1.x,
            x_local*sin(angle) + y1_local*cos(angle) + coord1.y),
        Coord2d(
            x_local*cos(angle) - y2_local*sin(angle) + coord1.x,
            x_local*sin(angle) + y2_local*cos(angle) + coord1.y)
    ]


def multilaterate(node_coords_3d, dists_3d, curr_depth, og_depth):
    """

    :param node_coords_3d: list of Coord3d of length 4
    :param dists_3d: list of floats of length 4
    :param curr_depth: float
    :param og_depth: float
    :return:
    """
    # get 2D projections
    dists_2d = np.array([convert_dist_to_2d(dist_3d, coord.z, curr_depth - og_depth)
                        for dist_3d, coord in zip(dists_3d, node_coords_3d)])

    # get 2D coordinates
    node_coords_2d = [Coord2d(coord.x, coord.y) for coord in node_coords_3d]

    # get points of intersection
    points_of_intersection = []
    for i in range(1, 4):
        for j in range(i+1, 4):
            points_of_intersection += get_points_of_intersection(
                node_coords_2d[i], node_coords_2d[j], dists_2d[i], dists_2d[j])

    # get the pair closest to each other and get the centroid
    min_dist = 6969696969
    idx_closest_points = []
    for i, point_i in enumerate(points_of_intersection):
        for j, point_j in enumerate(points_of_intersection[i+1:]):
            dist_between_points = sqrt((point_i.x - point_j.x)**2 + (point_i.y - point_j.y)**2)
            if dist_between_points < min_dist:
                min_dist = dist_between_points
                idx_closest_points = [i, j+i+1]
    centroid = Coord2d(
        (points_of_intersection[idx_closest_points[0]].x + points_of_intersection[idx_closest_points[1]].x) / 2,
        (points_of_intersection[idx_closest_points[0]].y + points_of_intersection[idx_closest_points[1]].y) / 2,
    )

    # get the closest point to the centroid and calculate the estimate
    i_closest = 0
    min_dist_to_centroid = 696966969696969
    for i in range(len(points_of_intersection)):
        if i not in idx_closest_points:
            dist_to_centroid = sqrt((centroid.x - points_of_intersection[i].x)**2 + (centroid.y - points_of_intersection[i].y)**2)
            if (dist_to_centroid < min_dist_to_centroid):
                min_dist_to_centroid = dist_to_centroid
                i_closest = i
    estimate = Coord2d(
        (points_of_intersection[i_closest].x + centroid.x) / 2,
        (points_of_intersection[i_closest].y + centroid.y) / 2,
    )

    return Coord3d(
        estimate.x,
        estimate.y,
        curr_depth - og_depth
    )


if __name__ == "__main__":
    origin_coord = Coord3d(0, 0, 0)
    stationary_1_coord = Coord3d(1, 0, 0)
    stationary_2_coords = [Coord3d(3, 2, -2), Coord3d(-1, 4, 2), Coord3d(0, 3, -2)]
    stationary_3_coords = [Coord3d(2, 5, -4), Coord3d(-4, 1, 0), Coord3d(-7, -2, 2), Coord3d(2, 5, -1)]
    rov_coords = [Coord3d(0, 0, 0), Coord3d(1, 1, 1), Coord3d(-1, 1, 2), Coord3d(-1, -1, -1), Coord3d(1, -1, -2)]
    og_depth = -5

    exact_tests_pass = True
    for stationary_2_coord in stationary_2_coords:
        for stationary_3_coord in stationary_3_coords:
            for rov_coord in rov_coords:
                node_coords_3d = [
                    origin_coord,
                    stationary_1_coord,
                    stationary_2_coord,
                    stationary_3_coord
                ]
                dists = [sqrt((node.x - rov_coord.x)**2 + (node.y - rov_coord.y)**2 + (node.z - rov_coord.z)**2)
                         for node in node_coords_3d]

                estimate = multilaterate(node_coords_3d, dists, og_depth + rov_coord.z, og_depth)

                diff = [fabs(estimate.x - rov_coord.x), fabs(estimate.y - rov_coord.y), fabs(estimate.z - rov_coord.z)]
                if any([d > 0.001 for d in diff]):
                    exact_tests_pass = False
                    print(f"actual: {rov_coord.x, rov_coord.y, rov_coord.z}, ", end="")
                    print(f"estimate: {estimate.x, estimate.y, estimate.z}, ", end="")
                    print(f"coords: [{origin_coord.x, origin_coord.y, origin_coord.z}, ", end="")
                    print(f"{stationary_1_coord.x, stationary_1_coord.y, stationary_1_coord.z}, ", end="")
                    print(f"{stationary_2_coord.x, stationary_2_coord.y, stationary_2_coord.z}, ", end="")
                    print(f"{stationary_3_coord.x, stationary_3_coord.y, stationary_3_coord.z}]")
    if exact_tests_pass:
        print("Exact tests passed")
    else:
        print("Exact tests failed")

    # approximate tests
    approximate_tests_pass = True
    variance = 0.2
    estimate_errors = []
    dist_errors = []
    for stationary_2_coord in stationary_2_coords:
        for stationary_3_coord in stationary_3_coords:
            for rov_coord in rov_coords:
                node_coords_3d = [
                    origin_coord,
                    stationary_1_coord,
                    stationary_2_coord,
                    stationary_3_coord
                ]
                dists = np.array(
                    [sqrt((node.x - rov_coord.x)**2 + (node.y - rov_coord.y)**2 + (node.z - rov_coord.z)**2)
                     for node in node_coords_3d])

                for _ in range(10000):
                    approx_dists = np.maximum(dists + np.random.randn(len(dists)) * variance, 0)
                    estimate = multilaterate(node_coords_3d, approx_dists, og_depth + rov_coord.z, og_depth)

                    approximation_error = np.sqrt(np.sum(np.square(dists - approx_dists)))
                    diff = sqrt((estimate.x - rov_coord.x) ** 2 + (estimate.y - rov_coord.y) ** 2 + (
                                estimate.z - rov_coord.z) ** 2)
                    max_diff = max([
                        fabs(estimate.x - rov_coord.x),
                        fabs(estimate.y - rov_coord.y),
                        fabs(estimate.z - rov_coord.z),
                    ])

                    estimate_errors += [
                        (estimate.x - rov_coord.x),
                        (estimate.y - rov_coord.y),
                        (estimate.z - rov_coord.z),
                    ]
                    dist_errors += [
                        approx_dists - dists
                    ]

    dist_errors = np.array(dist_errors)
    dist_errors = np.reshape(dist_errors, -1)
    dist_errors = np.random.choice(dist_errors, len(estimate_errors), replace=False)

    # Set the number of bins and the bin range
    num_bins = 1000
    bin_range = (-1.5, 1.5)

    # Compute the histogram for each data set
    hist1, bin_edges = np.histogram(estimate_errors, range=bin_range, bins=num_bins)
    hist2, _ = np.histogram(dist_errors, range=bin_range, bins=num_bins)

    # Set the same bin edges for all histograms
    plt.hist([estimate_errors, dist_errors], range=bin_range, bins=num_bins)

    # Add labels to the plot
    plt.legend(["estimate error", "dist error"])
    plt.xlabel("Error in a single direction")
    plt.ylabel("Frequency")

    # define the axis limits
    plt.ylim([0, 20000])
    plt.xlim([-1, 1])

    # Display the plot
    plt.show()
