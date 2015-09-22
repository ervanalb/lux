import random
import math

class Perlin2D:
    LKUP_SIZE = 1024

    @staticmethod
    def mod(x,y):
        return ((x % y) + x) % y

    @staticmethod
    def neighbors(xy):
        x = int(xy[0])
        y = int(xy[1])
        return ((x, y), (x + 1, y), (x + 1, y + 1), (x, y + 1))

    @staticmethod
    def cross_fade(t):
        return (3 - 2 * t) * t * t

    def __init__(self,period=(None,None),seed=None):
        self.period=period
        self.p=list(range(self.LKUP_SIZE))
        random.seed(seed)
        random.shuffle(self.p)

        def random_vector():
            return (math.sin(random.random()*2*math.pi),
                    math.cos(random.random()*2*math.pi))

        self.lut = [random_vector() for i in range(self.LKUP_SIZE)]

    def gradient(self, xy):
        (x, y) = xy
        (xp, yp) = self.period
        if xp is not None:
            x = self.mod(x, xp)
        if yp is not None:
            y = self.mod(y, yp)
        return self.lut[self.mod(x + self.p[self.mod(y, self.LKUP_SIZE)], self.LKUP_SIZE)]

    def __call__(self, pxy):
        ns = self.neighbors(pxy)

        def get_contribution(qxy):
            (gx, gy) = self.gradient(qxy)
            return (gx * (pxy[0] - qxy[0]) + gy * (pxy[1] - qxy[1])) * self.cross_fade(1 - abs(pxy[0] - qxy[0])) * self.cross_fade(1 - abs(pxy[1] - qxy[1]))

        return sum(map(get_contribution, ns))

class Perlin3D(Perlin2D):
    LKUP_SIZE = 1024

    @staticmethod
    def mod(x,y):
        return ((x % y) + x) % y

    @staticmethod
    def neighbors(xyz):
        x=int(xyz[0])
        y=int(xyz[1])
        z=int(xyz[2])
        return ((x, y, z),
                (x + 1, y, z),
                (x + 1, y + 1, z),
                (x, y + 1, z),
                (x, y, z + 1),
                (x + 1, y, z + 1),
                (x + 1, y + 1, z + 1),
                (x, y + 1, z + 1))

    def __init__(self, period = (None,None,None), seed = None):
        self.period=period
        self.p=range(self.LKUP_SIZE)
        random.seed(seed)
        random.shuffle(self.p)

        def random_vector():
            while True:
                x=random.random()
                y=random.random()
                z=random.random()

                if x*x+y*y+z*z < 1:
                    norm = math.sqrt(x*x + y*y + z*z)
                    return (x/norm, y/norm, z/norm)

        self.lut = [random_vector() for i in range(self.LKUP_SIZE)]

    def gradient(self, xyz):
        (x, y, z) = xyz
        (xp, yp, zp) = self.period
        if xp is not None:
            x = self.mod(x, xp)
        if yp is not None:
            y = self.mod(y, yp)
        if zp is not None:
            z = self.mod(z, zp)
        return self.lut[self.mod(x + self.p[self.mod(y + self.p[self.mod(z, self.LKUP_SIZE)], self.LKUP_SIZE)], self.LKUP_SIZE)]

    def __call__(self,pxyz):
        ns = self.neighbors(pxyz)

        def get_contribution(qxyz):
            (gx, gy, gz) = self.gradient(qxyz)
            return (gx * (pxyz[0] - qxyz[0]) + gy * (pxyz[1] - qxyz[1]) + gz * (pxyz[2] - qxyz[2])) \
                * self.cross_fade(1 - abs(px - qx)) \
                * self.cross_fade(1 - abs(py - qy)) \
                * self.cross_fade(1 - abs(pz - qz))

        return sum(map(get_contribution,ns))


if __name__ == "__main__":
    import matplotlib.pyplot as plt
    import time

    n2d=Perlin2D(period=(20,None))

    def curve(t):
        return [n2d((float(i)/20,t)) for i in range(0,200)]

    t=0

    plt.ion()
    fig, ax = plt.subplots()
    line, = ax.plot(curve(t))

    ax.relim()
    ax.autoscale_view()

    plt.show()

    while True:
        t+=0.01
        line.set_ydata(curve(t))
        ax.figure.canvas.draw()
        #time.sleep(0.01)
