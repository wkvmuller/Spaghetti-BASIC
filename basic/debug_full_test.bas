10 REM === VARIABLES, EXPRESSIONS, ASSIGNMENTS ===
20 LET A = 5
30 LET B = A * 2 + 10
40 PRINT "A=", A, "B=", B
50 PRINT VARS

60 REM === INPUT, READ, DATA, RESTORE ===
70 INPUT X
80 DATA 100, 200, 300
90 READ D1, D2
100 PRINT "D1=", D1, "D2=", D2
110 RESTORE
120 READ D3
130 PRINT "D3=", D3

140 REM === FILE AND SOUND ===
150 FILE WRITE "test_output.txt"
160 SOUND 440, 250

170 REM === DEBUGGING, TRACE, BREAK ===
180 DEBUG
190 LET C = A + B
200 TRACE C
210 BREAK
220 NODEBUG

230 REM === FINAL DUMP ===
240 PRINT VARS
250 END
