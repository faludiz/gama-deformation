A = [151,  89];
B = [ 32, 120];
C = [ 80,  65];

Q = (A + B + C)/3

QA = (A-Q);
QB = (B-Q);
QC = (C-Q);
dQA = norm(QA)
dQB = norm(QB)
dQC = norm(QC)

Q = Q + [0.02 0.01]

QA = (A-Q);
QB = (B-Q);
QC = (C-Q);
dQA = norm(QA)
dQB = norm(QB)
dQC = norm(QC)


# atan2(QA(1),QA(2))
