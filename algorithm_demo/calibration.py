from matplotlib import pyplot as plt, animation
import numpy as np


class Calibration:
    def __init__(self, coordinates: np.ndarray, speed: float) -> None:
        self.coords = coordinates
        self.speed = speed
        self.colors = ['blue', 'red', 'green', 'fuchsia']
        self.size = [200, 200, 200, 200]

        self.ax = plt.axes(xlim=(-10, 10), ylim=(-10, 10))

        self.patches = []
        for i in range(1, coordinates.shape[0]):
            self.patches.append(plt.Circle(coordinates[0], 0, fill=False, color=self.colors[i]))
            self.patches.append(plt.Circle(coordinates[i], 0, fill=False, color=self.colors[i]))

        self.i_start = self.get_i_start()

    def dist(self, p1, p2):
        return np.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1])**2)

    def get_i_start(self):
        i_start = []
        t_total = 0
        prev = self.coords[0]
        for idx, patch in enumerate(self.patches):
            if idx > 0 and idx % 2 == 0:
                t_total += 20
            coord = patch.center
            dist = self.dist(coord, prev)
            t_total += np.ceil(dist / self.speed)
            i_start.append(t_total)
            prev = np.copy(coord)
        return i_start

    def initialize_figure(self):
        for i in range(self.coords.shape[0]):
            self.ax.scatter(self.coords[i, 0], self.coords[i, 1], c=self.colors[i], s=self.size[i])

        plt.axis('off')
        plt.tight_layout()

        for patch in self.patches:
            self.ax.add_patch(patch)
        return self.patches

    def animate(self, i):
        for patch, i_start in zip(self.patches, self.i_start):
            if i >= i_start:
                patch.radius += self.speed
        return self.patches


fig = plt.figure(figsize=(10, 10))
coordinates = np.array([
    [0, 0],
    [8, -2],
    [-9, 9],
    [-3, -4],
])
a = Calibration(coordinates, 0.4)
gif = animation.FuncAnimation(fig, a.animate,
                               init_func=a.initialize_figure,
                               frames=300,
                               interval=0.1,
                               blit=True)

gif.save("calibration.gif")


