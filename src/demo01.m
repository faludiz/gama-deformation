A = [151,  89];
B = [ 32, 120];
C = [ 80,  65];

X = (A + B + C)/3

XA = (A-X);
XB = (B-X);
XC = (C-X);
dXA = norm(XA)
dXB = norm(XB)
dXC = norm(XC)

X = X + [0.02 0.01]

XA = (A-X);
XB = (B-X);
XC = (C-X);
dXA = norm(XA)
dXB = norm(XB)
dXC = norm(XC)


# atan2(XA(1),XA(2))
