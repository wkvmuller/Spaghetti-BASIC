10 REM --- MATRIX LU TEST ---
20 DIM A(3,3), B(3,3)
30 MAT READ A
40 MAT LU B = A
50 MAT PRINT #1, B_L
60 MAT PRINT #1, B_U
70 MAT MULT X = B_L * B_U
80 MAT PRINT #1, X
90 MAT E = A - X
100 MAT PRINT #1, E

110 REM --- RANK / INVERSE / SOLVE TEST ---
120 DIM INV(3,3), ID(3,3), RHS(3,1), X(3,1), ERR(3,1)
130 MAT READ RHS

140 LET R = RANK(A)
150 PRINT R

160 MAT INV = INVERSE(A)
170 MAT PRINT #1, INV

180 MAT MULT ID = A * INV
190 MAT PRINT #1, ID

200 MAT SOLVE X = A \ RHS
210 MAT PRINT #1, X

220 MAT MULT ERR = A * X
230 MAT PRINT #1, ERR

240 END

1000 DATA 2, 1, 1
1010 DATA 4, -6, 0
1020 DATA -2, 7, 2
1030 DATA 1, 2, 3
