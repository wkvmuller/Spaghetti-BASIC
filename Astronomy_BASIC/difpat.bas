10 REM   DIFFRACTION PATTERN
20 REM
40 DIM I(40,24),A(40,24),B(40,24)
50 R=3: REM  6-INCH APERTURE
60 L=.000022: REM  WAVELENGTH
70 GOSUB 530  
80 INPUT "HOW MANY RAYS";H
90 FOR I1=1 TO H
100 X=R*(2*RND-1)
110 Y=R*(2*RND-1)
115 REM NOTE THAT THE RND STATEMENT OPERATES
117 REM DIFFERENTLY ON DIFFERENT MACHINES
120 GOSUB 390  
130 IF F=0 THEN 100
140 PRINT "DOING RAY ";I1
150 GOSUB 450  
160 NEXT
170 REM
180 PRINT "NOW FINDING INTENSITY"
190 C=I(20,12)^2
200 FOR I=0 TO 40: FOR J=0 TO 24
210 I(I,J)=I(I,J)*I(I,J)/C
220 NEXT J: NEXT I
230 REM
240 REM   PRINT PICTURE
250 REM
260 INPUT "PRINTER ON";Q$
270 I$=" ./:;XH8M#": GOSUB 620  
280 FOR J=0 TO 24: LPRINT "I";
290 FOR I=0 TO 40
300 V=INT(9.99*I(I,J)^.47)+1
310 LPRINT MID$(I$,V,1);
320 NEXT I
330 LPRINT "I": NEXT J
340 GOSUB 620  
350 LPRINT
360 LPRINT "1 ARC SEC: I---------I"
370 GOTO 720  
380 REM
390 REM   APERTURE FILTER
400 REM
410 F=1
420 R1=SQR(X*X+Y*Y)
430 IF R1>R THEN F=0
440 RETURN
450 REM   PHASE CALCULATION
460 REM
470 FOR I=0 TO 40
480 FOR J=0 TO 24
490 P=X*A(I,J)+Y*B(I,J)
500 I(I,J)=I(I,J)+COS(P)
510 NEXT J: NEXT I
520 RETURN
530 REM   COMPUTE COEFFICIENTS
540 REM
550 K=2*3.14159265#/(L*206265!)
560 FOR I=0 TO 40: FOR J=0 TO 24
570 A(I,J)=K*(I-20)/10
580 B(I,J)=K*(12-J)/6
590 NEXT J: NEXT I
600 RETURN
610 REM
620 REM  PRINT A LINE
630 LPRINT " ";
640 FOR I=0 TO 40: LPRINT "-";
650 NEXT: LPRINT
660 RETURN
670 REM  =======================
680 REM  FROM "ASTRONOMICAL
690 REM  COMPUTING," SKY & TELE-
700 REM  SCOPE, SEPTEMBER, 1987
710 REM  =======================
720 END
