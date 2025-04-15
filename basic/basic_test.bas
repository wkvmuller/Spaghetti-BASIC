10 REM --- Variable assignments and math ---
20 LET A = 2
30 LET B = 3
40 LET C = POW(A, B)
50 LET D = ROUND(3.7)
60 LET E = FLOOR(3.7)
70 LET F = CEIL(3.1)
80 LET G = INT(9.8)
90 LET H = ASCII("Z")
100 LET I$ = CHR$(65)
110 LET J$ = STRING$(123)
120 LET K = VALUE("456")
130 LET L = TEST$("1234")
140 LET M = TIME()
150 LET N$ = TIME$()
160 LET O$ = DATE$()

170 REM --- Print statements and string functions ---
180 PRINT "Hello, World!"
190 PRINT A, B, C, D, E, F, G, H
200 PRINT I$, J$, K, L, N$, O$
210 PRINT LEN("Test"), LEFT$("Test", 2), RIGHT$("Test", 2), MID$("Test", 2, 2)

220 REM --- Control flow ---
230 IF A < B THEN 260
240 GOTO 280
250 STOP
260 PRINT "A is less than B"
270 GOTO 300
280 PRINT "This should not print"
290 STOP

300 REM --- Loops ---
310 FOR I = 1 TO 3
320 PRINT "FOR Loop:", I
330 NEXT I
340 LET X = 0
350 WHILE X < 3
360 PRINT "WHILE Loop:", X
370 LET X = X + 1
380 WEND
390 REPEAT
400 PRINT "REPEAT Loop"
410 LET X = X - 1
420 UNTIL X = 0

430 REM --- MAT operations ---
440 DIM A(2,3)
450 MAT READ A
460 DATA 1,2,3,4,5,6
470 MAT PRINT A

480 REM --- File operations ---
490 OPEN "testout.txt" FOR OUTPUT AS #1
500 PRINT #1, "Hello File"
510 CLOSE #1

520 REM --- Format printing ---
530 999 := "###.##"
540 PRINT USING 999, 123.456
550 PRINT #1 USING 999, 789.123

560 REM --- Other statements ---
570 SEED 12345
580 BEEP
590 END
