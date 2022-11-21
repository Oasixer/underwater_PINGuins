from matplotlib import pyplot as plt, animation
import numpy as np


class Calibration:
    def __init__(self, coordinates: np.ndarray, speed: float) -> None:
        self.coords = coordinates
        self.speed = speed
        self.colors = ['blue', 'red', 'green', 'fuchsia']
        self.size = [200, 200, 200, 200]

        self.ax = plt.axes(xlim=(-10, 10), ylim=(-10, 10))

        self.rov = plt.Circle(coordinates[0], radius=0.2, color=self.colors[0])
        self.ax.add_patch(self.rov)

        self.patches = []
        for i in range(coordinates.shape[0]):
            self.patches.append(plt.Circle(coordinates[i], 0, fill=False, color=self.colors[i]))

        self.i_start = self.get_i_start()

    def dist(self, p1, p2):
        return np.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1])**2)

    def get_i_start(self):
        i_start = []
        for patch in self.patches:
            dist = self.dist(patch.center, self.coords[0])
            i_start.append(np.ceil(dist / self.speed))
        return i_start

    def initialize_figure(self):
        for i in range(1, self.coords.shape[0]):
            self.ax.scatter(self.coords[i, 0], self.coords[i, 1], c=self.colors[i], s=self.size[i])

        plt.tight_layout()

        for patch in self.patches:
            self.ax.add_patch(patch)

        # Move left y-axis and bottim x-axis to centre, passing through (0,0)
        self.ax.spines['left'].set_position('center')
        self.ax.spines['bottom'].set_position('center')

        # Eliminate upper and right axes
        self.ax.spines['right'].set_color('none')
        self.ax.spines['top'].set_color('none')

        # Show ticks in the left and lower axes only
        self.ax.xaxis.set_ticks_position('bottom')
        self.ax.yaxis.set_ticks_position('left')
        return self.patches

    def animate(self, i):
        for patch, i_start in zip(self.patches, self.i_start):
            if i >= i_start:
                patch.radius += self.speed
        return self.patches


fig = plt.figure(figsize=(10, 10))
coordinates = np.array([
    [-7, 7],
    [8, -2],
    [-9, 9],
    [-3, -4],
])
a = Calibration(coordinates, 1)
gif = animation.FuncAnimation(fig, a.animate,
                               init_func=a.initialize_figure,
                               frames=80,
                               interval=0.1,
                               blit=True)

gif.save("call_and_response2.gif")


