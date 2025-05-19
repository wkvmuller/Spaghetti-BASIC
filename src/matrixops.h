// matrixops.h
#ifndef MATRIXOPS_H
#define MATRIXOPS_H

#include <vector>

struct Matrix {
    int rows, cols;
    std::vector<std::vector<double>> data;

    Matrix(int r = 0, int c = 0);
    double& operator()(int r, int c);
    double operator()(int r, int c) const;
};

Matrix scalarOp(const Matrix &m, double scalar, char op);
Matrix elementWiseOp(const Matrix &a, const Matrix &b, char op);
Matrix power(const Matrix &m, int exponent);
Matrix diagonal(const Matrix &m);
Matrix rankMatrix(const Matrix &m);
Matrix solve(const Matrix &A, const Matrix &B);
Matrix identityMatrixWrapper(int size);

Matrix multiplyMatrices(const Matrix &a, const Matrix &b);
Matrix determinantMatrix(const Matrix &m);
int computeRank(const Matrix &m);
Matrix solveLinearSystem(const Matrix &A, const Matrix &B);
Matrix identityMatrix(int size);

Matrix executeMATOperation(int opcode, const Matrix &A, const Matrix &B, double scalar = 0.0, int intArg = 0);

#endif // MATRIXOPS_H
