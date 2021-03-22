
.lib '/homes/oaramoon/PGD/sm046005-1f.lib' typical
.GLOBAL VSS VDD
.param vp =3.3
vvdd VDD 0 vp
vvss VSS 0 0
.option post nomod
.option posttop
.param c_load = 1f
M1 vdd a out vdd pmos_3p3 l=0.8u w=0.8u
M2 out out a vdd pmos_3p3 l=2.8u w=0.8u
M3 vss vdd vss vdd pmos_3p3 l=4u w=1.2u
M4 vss vss vdd vdd pmos_3p3 l=1.2u w=2u
M5 vdd a vss vdd pmos_3p3 l=0.8u w=2.4u
M6 vss vss out vss nmos_3p3 l=2.8u w=3.2u
M7 out a vss vss nmos_3p3 l=2.4u w=2u
M8 a out b vss nmos_3p3 l=1.6u w=2.8u
va a 0  pwl  0 0 ,4.9n 0, 5n vp , 9.9n vp, 10n 0, 14.9n 0,15n vp,20n vp
vb b 0  pwl  0 0 ,9.9n 0, 10n vp ,  20n vp
.tran 0.1n 20n sweep vp 1.2 3.3 0.3
.measure tran charge1 avg  v(out) from = 1n   to = 4.8n
.measure tran charge2 avg  v(out) from = 6n   to = 9.8n
.measure tran charge3 avg  v(out) from = 11n  to = 14.8n
.measure tran charge4 avg  v(out) from = 16n  to = 19.8n
.end
