10 REM === BASIC MATH, EXPRESSIONS, AND ASSIGNMENTS ===
20 LET A = 10
30 LET B = 20
40 LET C = A + B * 2
50 PRINT "C =", C

60 REM === DEG/RAD TRIG MODE TOGGLE ===
70 DEG
80 LET S = SIN(30)
90 LET C1 = COS(60)
100 RAD
110 LET T = TAN(1)
120 PRINT "SIN(30) DEG =", S, "COS(60) DEG =", C1, "TAN(1) RAD =", T

130 REM === LOG/EXP/POW AND CONVERSIONS ===
140 LET L = LOG(2.718)
150 LET L10 = LOG10(100)
160 LET E = EXP(1)
170 LET P = POW(2, 8)
180 LET D2R = DEG2RAD(180)
190 LET R2D = RAD2DEG(3.14159)
200 PRINT "LOG=", L, "LOG10=", L10, "EXP(1)=", E, "2^8=", P
210 PRINT "DEG2RAD(180)=", D2R, "RAD2DEG(PI)=", R2D

220 REM === ARC FUNCTIONS ===
230 DEG
240 LET A1 = ASIN(0.5)
250 LET A2 = ACOS(0.5)
260 LET A3 = ATAN(1)
270 PRINT "ASIN(0.5)=", A1, "ACOS(0.5)=", A2, "ATAN(1)=", A3

280 REM === DATA/READ/RESTORE ===
290 DATA 5,10,15
300 READ D1, D2
310 PRINT "READ:", D1, D2
320 RESTORE
330 READ D3
340 PRINT "RESTORED:", D3

350 REM === INPUT/TRACE/DEBUG/BREAK ===
360 INPUT X
370 DEBUG
380 LET Y = X * 2
390 TRACE Y
400 BREAK
410 NODEBUG

420 REM === DEF FN AND MAT ===
430 DEF FN SQR(X) = POW(X, 0.5)
440 LET SQ = FN SQR(49)
450 PRINT "SQRT(49)=", SQ
460 MAT A READ 1,2,3,4

470 REM === FILE: WRITE AND READ ===
480 OPEN #test_output.txt FOR OUTPUT AS #1
490 PRINTFILE #1 "RESULT=", SQ
500 CLOSE #1

510 OPEN #test_output.txt FOR INPUT AS #2
520 READ#2 TXT, SQVAL
530 PRINT "READ FROM FILE:", TXT, SQVAL
540 CLOSE #2

550 REM === ON GOTO/GOSUB ===
560 LET IDX = 2
570 ON IDX GOTO 600, 610, 620
580 PRINT "FALLBACK LINE"
590 GOTO 630
600 PRINT "GOTO TARGET 1"
610 PRINT "GOTO TARGET 2"
620 PRINT "GOTO TARGET 3"
630 END
