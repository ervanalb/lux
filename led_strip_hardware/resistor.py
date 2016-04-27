decade=[100.,120.,150.,180.,220.,270.,330.,390.,470.,560.,680.,820.]
values=decade+[v/10 for v in decade]+[v*10 for v in decade]

print "Ratio?"
r=raw_input()
r=float(r)

l=[]
for v in values:
    for w in values:
        l.append((v,w,float(v)/(v+w)))

print sorted(l,key=lambda (v,w,ratio):abs(ratio-r))[0:5]
