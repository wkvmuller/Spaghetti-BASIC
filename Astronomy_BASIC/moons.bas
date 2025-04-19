10 REM    NEW AND FULL MOONS
12 REM
14 REM
16 R1=3.14159265/180: U=0
18 INPUT "YEAR ";Y
20 PRINT
22 K0=INT((Y-1900)*12.3685)
24 T=(Y-1899.5)/100
26 T2=T*T: T3=T*T*T
28 J0=2415020+29*K0
30 F0=0.0001178*T2-0.000000155*T3
32 F0=F0+0.75933+0.53058868*K0
34 F0=F0-0.000837*T-0.000335*T2
36 J=J+INT(F): F=F-INT(F)
38 M0=K0*0.08084821133
40 M0=360*(M0-INT(M0))+359.2242
42 M0=M0-0.0000333*T2
44 M0=M0-0.00000347*T3
46 M1=K0*0.07171366128
48 M1=360*(M1-INT(M1))+306.0253
50 M1=M1+0.0107306*T2
52 M1=M1+0.00001236*T3
54 B1=K0*0.08519585128
56 B1=360*(B1-INT(B1))+21.2964
58 B1=B1-0.0016528*T2
60 B1=B1-0.00000239*T3
62 FOR K9=0 TO 28
64 J=J0+14*K9: F=F0+0.765294*K9
66 K=K9/2
68 M5=(M0+K*29.10535608)*R1
69 M6=(M1+K*385.81691806)*R1
70 B6=(B1+K*390.67050646)*R1
71 F=F-0.4068*SIN(M6)
72 F=F+(0.1734-0.000393*T)*SIN(M5)
73 F=F+0.0161*SIN(2*M6)
74 F=F+0.0104*SIN(2*B6)
75 F=F-0.0074*SIN(M5-M6)
76 F=F-0.0051*SIN(M5+M6)
77 F=F+0.0021*SIN(2*M5)
78 F=F+0.0010*SIN(2*B6-M6)
82 J=J+INT(F): F=F-INT(F)
84 IF U=0 THEN PRINT " NEW MOON ";
86 IF U=1 THEN PRINT "FULL MOON ";
88 PRINT J;F
90 U=U+1: IF U=2 THEN U=0
92 NEXT
94 END
95 REM  ------------------------
96 REM  APPEARED IN ASTRONOMICAL
97 REM  COMPUTING, SKY & TELE-
98 REM  SCOPE, MARCH, 1985
99 REM  ------------------------
