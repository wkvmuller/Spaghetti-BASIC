#ifndef MATRIXOPS_H
#define MATRIXOPS_H

#include <string>
#include <vector>
#include "program_structure.h"  // defines MatrixValue, MatrixIndex, PROGRAM_STRUCTURE

//-----------------------------------------------------------------------------
// Core dispatcher: parses a MAT-line and executes the requested operation.
//   - line: the full source line, e.g. "MAT INV = INVERSE(A)"
//   - A, B: optional operand matrices (empty if not used by this operation)
//   - scalar: optional scalar for scalar ops
//   - intArg: optional integer argument (e.g. exponent for POWER)
// Returns a MatrixValue when the operation produces a matrix, or a 1×1
// matrix containing a scalar result (e.g. DETERMINANT, RANK).
//-----------------------------------------------------------------------------
MatrixValue executeMATOperation(
    const std::string &line,
    const MatrixValue &A = {},
    const MatrixValue &B = {},
    double scalar = 0.0,
    int intArg = 0
);

//-----------------------------------------------------------------------------
// DIM helper: ensures the runtime’s program.matrices[name] exists and is sized.
// Should be called from your DIM statement handler.
//-----------------------------------------------------------------------------
void executeDIM(const std::string &line);

//-----------------------------------------------------------------------------
// MAT READ / PRINT
//-----------------------------------------------------------------------------
void executeMATREAD(const std::string &line);
void executeMATPRINT(const std::string &line, std::ostream &out);
void executeMATPRINTFILE(const std::string &line);

//-----------------------------------------------------------------------------
// Basic element-wise and scalar operations
//-----------------------------------------------------------------------------
MatrixValue matElementWiseOp(
    const MatrixValue &A,
    const MatrixValue &B,
    char op              // '+', '-', '*', '/'
);

MatrixValue matScalarOp(
    const MatrixValue &A,
    double scalar,
    char op,            // '+', '-', '*', '/'
    bool scalarFirst    // true if scalar on left (scalar * A)
);

//-----------------------------------------------------------------------------
// Standard linear-algebra routines
//-----------------------------------------------------------------------------
MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B);
MatrixValue matPower   (const MatrixValue &A, int exponent);
MatrixValue matTranspose(const MatrixValue &A);
MatrixValue matInverse  (const MatrixValue &A);
MatrixValue matSolve    (const MatrixValue &A, const MatrixValue &B);
double      matDeterminant(const MatrixValue &A);
int         matRank      (const MatrixValue &A);
double      matTrace     (const MatrixValue &A);
void        matLU        (
    const MatrixValue &A,
    MatrixValue &L,
    MatrixValue &U
);

//-----------------------------------------------------------------------------
// Special constructors
//-----------------------------------------------------------------------------
MatrixValue matIdentity(int n);
MatrixValue matOnes    (int rows, int cols);
MatrixValue matZeros   (int rows, int cols);

#endif // MATRIXOPS_H
