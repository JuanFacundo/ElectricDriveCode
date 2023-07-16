fc = 1/10
fm = 1/10e-3

wc = 2*pi*fc
Ts = 1/fm

Wc = 2*pi*(fc/fm)
wcd = 2/Ts * tan(Wc/2)


a0 = ((Ts^2)*(wcd^2)) / (1 + (sqrt(2)*wcd*Ts/2) + wcd^2*Ts/2)

a1 = a0*2

a2 = a0


b0 = 1

b1 = ((wcd^2)*Ts - 2) / (1 + (sqrt(2)*wcd*Ts/2) + wcd^2*Ts/2)

b2 = (1 - (sqrt(2)*wcd*Ts/2) + wcd^2*Ts/2) / (1 + (sqrt(2)*wcd*Ts/2) + wcd^2*Ts/2)
