#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

namespace ExtendedMATOPS {

// True matrix multiplication
Matrix multiply(const Matrix &A, const Matrix &B) {
  size_t n = A.size(), m = B[0].size(), p = B.size();
  if (A.empty() || p != A[0].size())
    throw std::invalid_argument("Dimension mismatch");
  Matrix C(n, std::vector<double>(m, 0.0));
  for (size_t i = 0; i < n; ++i)
    for (size_t k = 0; k < p; ++k)
      for (size_t j = 0; j < m; ++j)
        C[i][j] += A[i][k] * B[k][j];
  return C;
}

// Matrix power (integer exponent)
Matrix power(const Matrix &A, int exp) {
  if (A.empty() || A.size() != A[0].size())
    throw std::invalid_argument("Power requires square matrix");
  size_t n = A.size();
  Matrix result = identity(n);
  if (exp < 0)
    throw std::invalid_argument("Negative exponent not supported");
  Matrix base = A;
  while (exp) {
    if (exp & 1)
      result = multiply(result, base);
    base = multiply(base, base);
    exp >>= 1;
  }
  return result;
}

// Sum of diagonal elements
double trace(const Matrix &A) {
  if (A.empty() || A.size() != A[0].size())
    throw std::invalid_argument("Trace requires square matrix");
  double t = 0;
  for (size_t i = 0; i < A.size(); ++i)
    t += A[i][i];
  return t;
}

// Rank via Gaussian elimination
int rank(Matrix A) {
  const double EPS = 1e-9;
  int n = A.size(), m = A.empty() ? 0 : A[0].size(), r = 0;
  for (int c = 0; c < m && r < n; ++c) {
    int pivot = r;
    for (int i = r; i < n; ++i)
      if (std::fabs(A[i][c]) > std::fabs(A[pivot][c]))
        pivot = i;
    if (std::fabs(A[pivot][c]) < EPS)
      continue;
    std::swap(A[r], A[pivot]);
    double div = A[r][c];
    for (int j = c; j < m; ++j)
      A[r][j] /= div;
    for (int i = 0; i < n; ++i)
      if (i != r) {
        double factor = A[i][c];
        for (int j = c; j < m; ++j)
          A[i][j] -= factor * A[r][j];
      }
    ++r;
  }
  return r;
}

// Solve A X = B using Gaussian elimination
Matrix solve(const Matrix &A, const Matrix &B) {
  size_t n = A.size();
  if (n == 0 || A[0].size() != n || B.size() != n)
    throw std::invalid_argument("Dimension mismatch for solve");
  // Form augmented
  Matrix aug(n, std::vector<double>(n + B[0].size()));
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j)
      aug[i][j] = A[i][j];
    for (size_t j = 0; j < B[0].size(); ++j)
      aug[i][n + j] = B[i][j];
  }
  // Forward eliminate
  for (size_t i = 0; i < n; ++i) {
    size_t pivot = i;
    for (size_t r = i; r < n; ++r)
      if (std::fabs(aug[r][i]) > std::fabs(aug[pivot][i]))
        pivot = r;
    if (std::fabs(aug[pivot][i]) < 1e-9)
      throw std::runtime_error("Singular matrix");
    std::swap(aug[i], aug[pivot]);
    double d = aug[i][i];
    for (size_t j = i; j < aug[i].size(); ++j)
      aug[i][j] /= d;
    for (size_t r = 0; r < n; ++r)
      if (r != i) {
        double f = aug[r][i];
        for (size_t j = i; j < aug[r].size(); ++j)
          aug[r][j] -= f * aug[i][j];
      }
  }
  // Extract solution
  Matrix X(n, std::vector<double>(B[0].size()));
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j < X[0].size(); ++j)
      X[i][j] = aug[i][n + j];
  return X;
}

// Identity, zeros, ones
Matrix identity(int n) {
  Matrix I(n, std::vector<double>(n, 0.0));
  for (int i = 0; i < n; ++i)
    I[i][i] = 1.0;
  return I;
}
Matrix zeros(int r, int c) { return Matrix(r, std::vector<double>(c, 0.0)); }
Matrix ones(int r, int c) { return Matrix(r, std::vector<double>(c, 1.0)); }

// LU Decomposition (Doolittle, no pivot)
std::pair<Matrix, Matrix> lu(const Matrix &A) {
  size_t n = A.size();
  if (n == 0 || A[0].size() != n)
    throw std::invalid_argument("LU requires square");
  Matrix L = identity(n), U = zeros(n, n);
  for (size_t i = 0; i < n; ++i) {
    for (size_t k = i; k < n; ++k) {
      double sum = 0;
      for (size_t j = 0; j < i; ++j)
        sum += L[i][j] * U[j][k];
      U[i][k] = A[i][k] - sum;
    }
    for (size_t k = i + 1; k < n; ++k) {
      double sum = 0;
      for (size_t j = 0; j < i; ++j)
        sum += L[k][j] * U[j][i];
      L[k][i] = (A[k][i] - sum) / U[i][i];
    }
  }
  return {L, U};
}

// QR Decomposition (Gram-Schmidt)
std::pair<Matrix, Matrix> qr(const Matrix &A) {
  size_t m = A.size(), n = A[0].size();
  Matrix Q = zeros(m, n), R = zeros(n, n);
  for (size_t k = 0; k < n; ++k) {
    // copy column
    for (size_t i = 0; i < m; ++i)
      Q[i][k] = A[i][k];
    for (size_t j = 0; j < k; ++j) {
      double dot = 0;
      for (size_t i = 0; i < m; ++i)
        dot += Q[i][j] * A[i][k];
      R[j][k] = dot;
      for (size_t i = 0; i < m; ++i)
        Q[i][k] -= dot * Q[i][j];
    }
    double norm = 0;
    for (size_t i = 0; i < m; ++i)
      norm += Q[i][k] * Q[i][k];
    norm = std::sqrt(norm);
    R[k][k] = norm;
    if (norm < 1e-9)
      throw std::runtime_error("Rank-deficient");
    for (size_t i = 0; i < m; ++i)
      Q[i][k] /= norm;
  }
  return {Q, R};
}

// SVD and Eigen: Not implemented
std::tuple<Matrix, Matrix, Matrix> svd(const Matrix &) {
  throw std::runtime_error("SVD not implemented");
}
std::pair<Matrix, std::vector<double>> eig(const Matrix &) {
  throw std::runtime_error("Eigen not implemented");
}

// Element-wise functions
Matrix exp(const Matrix &A) {
  Matrix B = A;
  for (auto &row : B)
    for (auto &v : row)
      v = std::exp(v);
  return B;
}
Matrix sin(const Matrix &A) {
  Matrix B = A;
  for (auto &row : B)
    for (auto &v : row)
      v = std::sin(v);
  return B;
}

// Concatenation
Matrix hconcat(const Matrix &A, const Matrix &B) {
  if (A.size() != B.size())
    throw std::invalid_argument("Row count mismatch");
  Matrix C = A;
  for (size_t i = 0; i < C.size(); ++i)
    C[i].insert(C[i].end(), B[i].begin(), B[i].end());
  return C;
}
Matrix vconcat(const Matrix &A, const Matrix &B) {
  if (A[0].size() != B[0].size())
    throw std::invalid_argument("Column count mismatch");
  Matrix C = A;
  C.insert(C.end(), B.begin(), B.end());
  return C;
}

// === Eigen‐based SVD and Eigen Decompositions ===
// Append this to the end of ExtendedMATOPS.hpp

#ifndef EXTENDED_MATOPS_EIGEN_HPP
#define EXTENDED_MATOPS_EIGEN_HPP

#include <Eigen/Dense>

namespace MATOPS {

// Compute the Singular Value Decomposition of any (dense) matrix `mat`.
// U   : Left singular vectors (size Rows×Rows)
// S   : Singular values (size min(Rows,Cols)×1)
// V   : Right singular vectors (size Cols×Cols)
template <typename Scalar, int Rows, int Cols>
void computeSVD(const Eigen::Matrix<Scalar, Rows, Cols> &mat,
                Eigen::Matrix<Scalar, Rows, Rows> &U,
                Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &S,
                Eigen::Matrix<Scalar, Cols, Cols> &V) {
  Eigen::JacobiSVD<Eigen::Matrix<Scalar, Rows, Cols>> svd(
      mat, Eigen::ComputeFullU | Eigen::ComputeFullV);
  U = svd.matrixU();
  S = svd.singularValues();
  V = svd.matrixV();
}

// Compute the Eigenvalue decomposition of any square matrix `mat`.
// eigenvalues   : Real parts of eigenvalues (size Dim×1)
// eigenvectors  : Corresponding real eigenvectors (size Dim×Dim)
template <typename Scalar, int Dim>
void computeEigen(const Eigen::Matrix<Scalar, Dim, Dim> &mat,
                  Eigen::Matrix<Scalar, Dim, 1> &eigenvalues,
                  Eigen::Matrix<Scalar, Dim, Dim> &eigenvectors) {
  Eigen::EigenSolver<Eigen::Matrix<Scalar, Dim, Dim>> solver(mat);
  eigenvalues = solver.eigenvalues().real();
  eigenvectors = solver.eigenvectors().real();
}

} // namespace MATOPS

#endif // EXTENDED_MATOPS_EIGEN_HPP

} // namespace ExtendedMATOPS
