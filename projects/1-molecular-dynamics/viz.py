
if __name__ == "__main__":
    import sys
    import json
    import numpy as np
    from matplotlib import pyplot as plt
    import mpl_toolkits.mplot3d.axes3d as p3
    from matplotlib import animation
    from mpl_toolkits.mplot3d.art3d import juggle_axes

    # from https://stackoverflow.com/questions/9401658/how-to-animate-a-scatter-plot#9416663
    class AnimatedScatter(object):
        """An animated scatter plot using matplotlib.animations.FuncAnimation."""
        def __init__(self, stdin=sys.stdin, numpoints=50, numsteps=1, L=1.):
            self.numpoints = numpoints
            self.stream = self.data_stream()
            self.stdin = stdin
            self.numsteps = numsteps
            self.L = L

            # Setup the figure and axes...
            self.fig = plt.figure()
            self.ax = p3.Axes3D(self.fig)
            # Then setup FuncAnimation.
            self.ani = animation.FuncAnimation(self.fig, self.update, frames=numsteps, interval=1,
                                               repeat=False,
                                               init_func=self.setup_plot)#, blit=True)

        def setup_plot(self):
            """Initial drawing of the scatter plot."""
            X = next(self.stream)
            self.scat = self.ax.scatter(X[0,:], X[1,:], X[2,:], c=np.array(range(X.shape[1])))#, animated=True)
            self.ax.set_xlim3d([-self.L/2.,self.L/2.])
            self.ax.set_ylim3d([-self.L/2.,self.L/2.])
            self.ax.set_zlim3d([-self.L/2.,self.L/2.])

            # For FuncAnimation's sake, we need to return the artist we'll be using
            # Note that it expects a sequence of artists, thus the trailing comma.
            return self.scat,

        def data_stream(self):
            while True:
                rline = self.stdin.readline()
                obj = json.loads(rline)
                try:
                    X = np.array(obj['X'])
                    for i in range(X.shape[0]):
                        for j in range(X.shape[1]):
                            X[i,j] = X[i,j] - round(X[i,j] / self.L) * self.L
                    yield X
                except:
                    break

        def update(self, i):
            """Update the scatter plot."""
            X = next(self.stream)

            # Set x and y data...
            self.scat._offsets3d = (np.ma.ravel(X[0,:]), np.ma.ravel(X[1,:]), np.ma.ravel(X[2,:]))
            # Set colors..
            # self.scat.set_array(range(X.shape[1]))

            # We need to return the updated artist for FuncAnimation to draw..
            # Note that it expects a sequence of artists, thus the trailing comma.
            #plt.draw()
            return self.scat,

        def show(self,filename=None):
            if filename:
                self.ani.save(f'{filename}.gif',dpi=80,writer='imagemagick')
            else:
                plt.show()

    num_proc = 0
    Np = 0
    Nt = 0
    Nint = 0
    k = 0.
    d = 0.
    dt = 0.
    s = 0
    gifname = None
    firstline = sys.stdin.readline()
    obj = json.loads(firstline)

    Np = obj['num_points']
    dt = obj['dt']
    L  = obj['L']
    Nt = obj['num_steps']
    Nint = obj['step_chunk']
    k = obj['k']
    d = obj['d']
    gifname = obj['gifname']

    numsteps = int(Nt) // int(Nint)
    a = AnimatedScatter(numpoints = Np, numsteps=numsteps, L=L)

    a.show(gifname)

