import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np

# creating a blank window
# for the animation
fig = plt.figure()
axis = plt.axes(xlim=(-10, 10), ylim=(-10, 10))

coordinates = np.array([
    [0, 0],
    [8, -4],
    [-2, 9],
    [-5, -7],
])
plt.scatter(coordinates[:, 0], coordinates[:, 1])
plt.axis('off')
plt.show()
