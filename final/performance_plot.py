import matplotlib
import matplotlib.pyplot as plt
import numpy as np

# Data for plotting
t =[19200, 38400,57600,76800,96000, 115200]
s =[768.33,879.20,920.17,937.13,957.17,973.16]

fig, ax = plt.subplots()
ax.plot(t, s,'-o')

ax.set(xlabel='Problem Size N', ylabel='GFlops',
       title='Single Node Performance')


fig.savefig("singlenode.png")
plt.show()