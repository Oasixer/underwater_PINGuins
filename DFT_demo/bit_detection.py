import numpy as np
import matplotlib.pyplot as plt
from math import sin, pi


t = [0, 2.5, 5, 7.5, 10]
x = [0, 0, 1, 0, 0]

plt.figure()
plt.plot(t, x)
plt.xlabel("time [ms]")
plt.ylabel("|X(freq)|")
plt.show()

t = [i/500000 for i in list(range(5000))]
t = np.array(t)
x = np.sin(10000*2*pi*t)
x[:1250] = 0
x[2500:] = 0

plt.figure()
plt.plot(t*1000, x)
plt.xlabel("time [ms]")
plt.ylabel("amplitude sent")
plt.show()