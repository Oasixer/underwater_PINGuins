from matplotlib import pyplot as plt, animation
import numpy as np


class Calibration:
    def __init__(self, coordinates: np.ndarray, start: np.ndarray, end: np.ndarray, frames: float) -> None:
        self.coords = coordinates
        self.start = start
        self.end = end
        self.speed = frames

        self.x_inc = (end[0] - start[0]) / frames
        self.y_inc = (end[1] - start[1]) / frames

        self.colors = ['blue', 'red', 'green', 'fuchsia']
        self.size = [200, 200, 200, 200]

        self.ax = plt.axes(xlim=(-10, 10), ylim=(-10, 10))

        self.rov = plt.Circle(start, radius=0.2, color=self.colors[0])

    def dist(self, p1, p2):
        return np.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1])**2)

    def initialize_figure(self):
        for i in range(1, self.coords.shape[0]):
            self.ax.scatter(self.coords[i, 0], self.coords[i, 1], c=self.colors[i], s=self.size[i])

        plt.tight_layout()

        self.ax.add_patch(self.rov)

        # Move left y-axis and bottim x-axis to centre, passing through (0,0)
        self.ax.spines['left'].set_position('center')
        self.ax.spines['bottom'].set_position('center')

        # Eliminate upper and right axes
        self.ax.spines['right'].set_color('none')
        self.ax.spines['top'].set_color('none')

        # Show ticks in the left and lower axes only
        self.ax.xaxis.set_ticks_position('bottom')
        self.ax.yaxis.set_ticks_position('left')
        return [self.rov]

    def animate(self, i):
        x, y = self.rov.center
        self.rov.center = x + self.x_inc, y + self.y_inc
        print(self.rov.center)
        return [self.rov]


fig = plt.figure(figsize=(10, 10))
coordinates = np.array([
    [5, 5],
    [8, -2],
    [-9, 9],
    [-3, -4],
])
frames = 40
a = Calibration(coordinates, np.array([5, 5]), np.array([-7, 7]), frames)
gif = animation.FuncAnimation(fig, a.animate,
                               init_func=a.initialize_figure,
                               frames=frames,
                               interval=0.1,
                               blit=True)

gif.save("moving2.gif")


