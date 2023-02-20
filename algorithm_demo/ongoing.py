from matplotlib import pyplot as plt, animation
import numpy as np


class Ongoing:
    def __init__(self, coordinates: np.ndarray, wave_speed: float, rov_positions: np.ndarray, rov_speed: float) -> None:
        self.coords = coordinates
        self.wave_speed = wave_speed
        self.rov_positions = rov_positions
        self.rov_speed = rov_speed

        self.colors = ['red', 'green', 'fuchsia']
        self.markers = [".", ".", "."]

        self.ax = plt.axes(xlim=(-10, 10), ylim=(-10, 10))

        self.responses = []
        for i in range(coordinates.shape[0]):
            self.responses.append(plt.Circle(coordinates[i], 0, fill=False, color=self.colors[i]))

        self.rov = plt.Circle((0, 0), radius=0.2, color='blue')

        self.call = plt.Circle(coordinates[0], 0, fill=False, color='blue')

        self.i_respond = {}
        for rov_pos in rov_positions:
            self.i_respond[rov_pos] = self.get_i_start(rov_pos)

        self.i_call = 0
        self.i_move = max(self.i_respond[self.rov_positions[0]]) + self.i_call
        self.i_rov_position = 0

    def dist(self, p1, p2):
        return np.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1])**2)

    def get_i_start(self, rov_pos):
        i_start = []
        for patch in self.responses:
            dist = self.dist(rov_pos, patch.center)
            i_start.append(np.ceil(dist / self.wave_speed))
        return i_start

    def initialize_figure(self):
        for i in range(self.coords.shape[0]):
            self.ax.scatter(self.coords[i, 0], self.coords[i, 1], c=self.colors[i], marker=self.markers[i], s=200)

        plt.tight_layout()

        for patch in self.responses:
            self.ax.add_patch(patch)
        self.ax.add_patch(self.call)
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
        return self.responses

    def animate(self, i):
        self.call.radius += self.wave_speed
        for patch, i_start in zip(self.responses, self.i_respond[self.call.center]):
            if i >= i_start:
                patch.radius += self.wave_speed

        x, y = self.rov.center
        self.rov.center = x + 0.1, y + 0.1
        return self.responses


fig = plt.figure(figsize=(10, 10))
coordinates = np.array([
    [8, -2],
    [-9, 9],
    [-3, -4],
])
rov_positions = np.array([
    [0, 0],
    [5, 5],
    [-5, 1],
])
a = Ongoing(coordinates, 1.5, rov_positions, 0.3)
gif = animation.FuncAnimation(fig, a.animate,
                               init_func=a.initialize_figure,
                               frames=30,
                               interval=0.1,
                               blit=True)

gif.save("ongoing.gif")


