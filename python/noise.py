import random
import math

class Perlin2D:
    LKUP_SIZE = 1024

    @staticmethod
    def mod(x,y):
        return ((x % y) + x) % y

    @staticmethod
    def neighbors((x,y)):
        x=int(x)
        y=int(y)
        return ((x,y),(x+1,y),(x+1,y+1),(x,y+1))

    @staticmethod
    def cross_fade(t):
        return (3 - 2 * t) * t * t

    def __init__(self,period=(None,None),seed=None):
        self.period=period
        self.p=range(self.LKUP_SIZE)
        random.seed(seed)
        random.shuffle(self.p)

        def random_vector():
            return (math.sin(random.random()*2*math.pi),
                    math.cos(random.random()*2*math.pi))

        self.lut = [random_vector() for i in range(self.LKUP_SIZE)]

    def gradient(self,(x,y)):
        xp,yp=self.period
        if xp is not None:
            x=self.mod(x,xp)
        if yp is not None:
            y=self.mod(y,yp)
        return self.lut[self.mod(x + self.p[self.mod(y, self.LKUP_SIZE)], self.LKUP_SIZE)]

    def __call__(self,(px,py)):
        ns = self.neighbors((px,py))

        def get_contribution((qx,qy)):
            (gx,gy) = self.gradient((qx,qy))
            return (gx * (px - qx) + gy * (py - qy)) * self.cross_fade(1 - abs(px - qx)) * self.cross_fade(1 - abs(py - qy))

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
