import lux
from noise import Perlin2D
import time
import colorsys

b = lux.Bus("/dev/ttyACM0")
with b:
    strip = lux.LEDStrip(b,0xFFFFFFFF)
    l = strip.length

    freq = 5

    ng = Perlin2D(period=(freq,None))

    def hsl_to_rgb(h,s,l):
        result = colorsys.hls_to_rgb(h,l,s)
        return tuple([int(round(255*c)) for c in result])

    st=time.time()
    def rescale_h(f):
        f = (f + 1) % 1
        return f

    def rescale_x(f):
        return f * freq

    def rescale_y(f):
        return t / 5

    while True:
        t=time.time()-st

        frame = [hsl_to_rgb(rescale_h(ng((rescale_x(float(i)/l),rescale_y(float(i)/l)))),1.0,0.5) for i in range(l)]
        print "send frame"
        strip.send_frame(frame)
        time.sleep(0.01)
