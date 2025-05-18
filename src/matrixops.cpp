#include "program_structure.h"
#include <cmath>
#include <stdexcept>
#include <vector>
#include <numeric>

// --- Helper prototypes for matrix operations ---
MatrixValue matElementWiseOp(const MatrixValue &A, const MatrixValue &B,
                             char op);
MatrixValue matScalarOp(const MatrixValue &A, double scalar, char op);
double matDeterminant(const MatrixValue &A);
MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B);
MatrixValue matPower(const MatrixValue &A, int exponent);
double matTrace(const MatrixValue &A);
int matRank(const MatrixValue &A);
MatrixValue matIdentity(const std::vector<int> &dims);
MatrixValue matZeros(const std::vector<int> &dims);
MatrixValue matOnes(const std::vector<int> &dims);
MatrixValue matZero(const MatrixValue &A);
void matLU(const MatrixValue &A, MatrixValue &L, MatrixValue &U);
MatrixValue matSolve(const MatrixValue &A, const MatrixValue &B);

// Dispatch all MAT assignment and operations
void executeMAT(const std::string &line) {
  // 1) Elementwise ops or scalar ops: MAT X = A + B  or  MAT X = A * 5
  static const std::regex assignRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*([-+*/])\s*([A-Z][A-Z0-9_]*|[0-9]+(?:\.[0-9]*)?)\s*$)",
      std::regex::icase);
  // 2) Determinant: MAT X = DETERMINANT(A)
  static const std::regex detRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*DETERMINANT\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  // 3) Matrix multiply: MAT MULT X = A * B
  static const std::regex multRe(
      R"(^\s*MAT\s+MULT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\*\s*([A-Z][A-Z0-9_]*)\s*$)",
      std::regex::icase);
  // 4) Power: MAT POWER X = A ^ n
  static const std::regex powRe(
      R"(^\s*MAT\s+POWER\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\^\s*([0-9]+)\s*$)",
      std::regex::icase);
  // 5) Trace: MAT TRACE X = TRACE(A)
  static const std::regex traceRe(
      R"(^\s*MAT\s+TRACE\s+([A-Z][A-Z0-9_]*)\s*=\s*TRACE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  // 6) Rank: MAT RANK X = RANK(A)
  static const std::regex rankRe(
      R"(^\s*MAT\s+RANK\s+([A-Z][A-Z0-9_]*)\s*=\s*RANK\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  // 7) Solve: MAT SOLVE X = A \ B
  static const std::regex solveRe(
      R"(^\s*MAT\s+SOLVE\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\\\s*([A-Z][A-Z0-9_]*)\s*$)",
      std::regex::icase);
  // 8) Special: ZERO, IDENTITY, ZEROS, ONES, LU
  static const std::regex zeroRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*ZERO\s*;\s*$)", std::regex::icase);
  static const std::regex idRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*IDENTITY\s*\(\s*(?:([0-9]+)(?:\s*,\s*([0-9]+))*)?\s*\)\s*$)",
      std::regex::icase);
  static const std::regex zerosRe(
      R"(^\s*MAT\s+ZEROS\s+([A-Z][A-Z0-9_]*)\s*=\s*ZEROS\s*\(\s*(.+)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex onesRe(
      R"(^\s*MAT\s+ONES\s+([A-Z][A-Z0-9_]*)\s*=\s*ONES\s*\(\s*(.+)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex luRe(
      R"(^\s*MAT\s+LU\s+([A-Z][A-Z0-9_]*)\s*=\s*LU\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);

  std::smatch m;
  if (std::regex_match(line, m, assignRe)) {
    std::string X = m[1], A = m[2], op = m[3], B = m[4];
    MatrixValue &mX = program.numericMatrices[X];
    MatrixValue &mA = program.numericMatrices[A];
    if (std::isdigit(B[0])) {
      double scalar = std::stod(B);
      mX = matScalarOp(mA, scalar, op[0]);
    } else {
      MatrixValue &mB = program.numericMatrices[B];
      mX = matElementWiseOp(mA, mB, op[0]);
    }
  } else if (std::regex_match(line, m, detRe)) {
    // MAT X = DETERMINANT(A)
    {
      VarInfo v;
      v.numericValue = matDeterminant(program.numericMatrices[m[2].str()]);
      v.stringValue  = "";
      v.isString     = false;
      program.numericVariables[m[1].str()] = v;
    }
  } else if (std::regex_match(line, m, multRe)) {
    program.numericMatrices[m[1]] = matMultiply(program.numericMatrices[m[2]],
                                                program.numericMatrices[m[3]]);
  } else if (std::regex_match(line, m, powRe)) {
    program.numericMatrices[m[1]] =
        matPower(program.numericMatrices[m[2]], std::stoi(m[3]));
  } else if (std::regex_match(line, m, traceRe)) {
    // MAT X = TRACE(A)
    {
      VarInfo v;
      v.numericValue = matTrace(program.numericMatrices[m[2].str()]);
      v.stringValue  = "";
      v.isString     = false;
      program.numericVariables[m[1].str()] = v;
    }
  } else if (std::regex_match(line, m, rankRe)) {
    // MAT X = RANK(A)
    {
      VarInfo v;
      v.numericValue = static_cast<double>(matRank(program.numericMatrices[m[2].str()]));
      v.stringValue  = "";
      v.isString     = false;
      program.numericVariables[m[1].str()] = v;
    }
  } else if (std::regex_match(line, m, solveRe)) {
    program.numericMatrices[m[1]] =
        matSolve(program.numericMatrices[m[2]], program.numericMatrices[m[3]]);
  } else if (std::regex_match(line, m, zeroRe)) {
    program.numericMatrices[m[1]] = matZero(program.numericMatrices[m[1]]);
  } else if (std::regex_match(line, m, idRe)) {
    std::vector<int> dims;
    for (size_t i = 2; i < m.size(); ++i)
      if (m[i].matched)
        dims.push_back(std::stoi(m[i]));
    program.numericMatrices[m[1]] = matIdentity(dims);
  } else if (std::regex_match(line, m, zerosRe)) {
    // args in m[2] as comma-separated dims
    std::vector<int> dims; /* parse m[2] */
    program.numericMatrices[m[1]] = matZeros(dims);
  } else if (std::regex_match(line, m, onesRe)) {
    std::vector<int> dims; /* parse m[2] */
    program.numericMatrices[m[1]] = matOnes(dims);
  } else if (std::regex_match(line, m, luRe)) {
    // MAT MULTIPLY / LU Decomp
    MatrixValue L, U;
    matLU(program.numericMatrices[m[2].str()], L, U);
    program.numericMatrices[m[1].str() + "_L"] = L;
    program.numericMatrices[m[1].str() + "_U"] = U;
  } else {
    throw std::runtime_error("SYNTAX ERROR: Unknown MAT operation: " + line);
  }
}

// Implementations of helpers would follow here, using dense vs sparse storage
// logic. Helper to compute linear index -> multidim index
static MatrixIndex linearToMulti(size_t idx, const std::vector<int> &dims) {
  int D = static_cast<int>(dims.size());
  MatrixIndex mi;
  mi.dimensions.resize(D);
  size_t tmp = idx;
  for (int d = D - 1; d >= 0; --d) {
    mi.dimensions[d] = static_cast<int>(tmp % dims[d]);
    tmp /= dims[d];
  }
  return mi;
}

// (1)-(3) Element-wise and scalar-matrix operations
MatrixValue matElementWiseOp(const MatrixValue &A, const MatrixValue &B,
                             char op) {
  if (A.dimensions != B.dimensions) {
    throw std::runtime_error(
        "Matrix dimension mismatch in element-wise operation");
  }
  MatrixValue C;
  C.dimensions = A.dimensions;
  size_t total = 1;
  for (int d : C.dimensions)
    total *= static_cast<size_t>(d);
  C.configureStorage(total);

  for (size_t idx = 0; idx < total; ++idx) {
    // fetch A
    double a = 0.0;
    if (!A.isSparse)
      a = A.denseValues[idx].numericValue;
    else {
      MatrixIndex mi = linearToMulti(idx, A.dimensions);
      auto it = A.sparseValues.find(mi);
      if (it != A.sparseValues.end())
        a = it->second.numericValue;
    }
    // fetch B
    double b = 0.0;
    if (!B.isSparse)
      b = B.denseValues[idx].numericValue;
    else {
      MatrixIndex mi = linearToMulti(idx, B.dimensions);
      auto it = B.sparseValues.find(mi);
      if (it != B.sparseValues.end())
        b = it->second.numericValue;
    }
    // compute
    double r;
    switch (op) {
    case '+':
      r = a + b;
      break;
    case '-':
      r = a - b;
      break;
    case '*':
      r = a * b;
      break;
    case '/':
      if (b == 0.0)
        throw std::runtime_error("Division by zero in element-wise matrix op");
      r = a / b;
      break;
    default:
      throw std::runtime_error("Unknown element-wise operator");
    }
    // store
    VarInfo vi;
    vi.isString = false;
    vi.numericValue = r;
    vi.isArray = false;
    if (!C.isSparse)
      C.denseValues[idx] = vi;
    else
      C.sparseValues[linearToMulti(idx, C.dimensions)] = vi;
  }
  return C;
}

MatrixValue matScalarOp(const MatrixValue &A, double scalar, char op,
                        bool scalarFirst) {
  MatrixValue C;
  C.dimensions = A.dimensions;
  size_t total = 1;
  for (int d : C.dimensions)
    total *= static_cast<size_t>(d);
  C.configureStorage(total);

  for (size_t idx = 0; idx < total; ++idx) {
    double a = 0.0;
    if (!A.isSparse)
      a = A.denseValues[idx].numericValue;
    else {
      MatrixIndex mi = linearToMulti(idx, A.dimensions);
      auto it = A.sparseValues.find(mi);
      if (it != A.sparseValues.end())
        a = it->second.numericValue;
    }
    double r;
    double x = scalarFirst ? scalar : a;
    double y = scalarFirst ? a : scalar;
    switch (op) {
    case '+':
      r = x + y;
      break;
    case '-':
      r = x - y;
      break;
    case '*':
      r = x * y;
      break;
    case '/':
      if (y == 0.0)
        throw std::runtime_error("Division by zero in scalar-matrix op");
      r = x / y;
      break;
    default:
      throw std::runtime_error("Unknown scalar-matrix operator");
    }
    VarInfo vi;
    vi.isString = false;
    vi.numericValue = r;
    vi.isArray = false;
    if (!C.isSparse)
      C.denseValues[idx] = vi;
    else
      C.sparseValues[linearToMulti(idx, C.dimensions)] = vi;
  }
  return C;
}

// (5) Matrix multiplication
MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B) {
  // Only dense multiplication supported
  if (A.dimensions.size() != 2 || B.dimensions.size() != 2)
    throw std::runtime_error("Matrix multiply requires 2D matrices");
  int rA = A.dimensions[0], cA = A.dimensions[1];
  int rB = B.dimensions[0], cB = B.dimensions[1];
  if (cA != rB)
    throw std::runtime_error("Dimension mismatch in matrix multiplication");

  MatrixValue C;
  C.dimensions = {rA, cB};
  size_t total = static_cast<size_t>(rA) * cB;
  C.configureStorage(total);

  for (int i = 0; i < rA; ++i) {
    for (int j = 0; j < cB; ++j) {
      double sum = 0.0;
      for (int k = 0; k < cA; ++k) {
        size_t idxA = static_cast<size_t>(i) * cA + k;
        size_t idxB = static_cast<size_t>(k) * cB + j;
        double vA =
            A.isSparse
                ? (A.sparseValues.count(linearToMulti(idxA, A.dimensions))
                       ? A.sparseValues.at(linearToMulti(idxA, A.dimensions))
                             .numericValue
                       : 0.0)
                : A.denseValues[idxA].numericValue;
        double vB =
            B.isSparse
                ? (B.sparseValues.count(linearToMulti(idxB, B.dimensions))
                       ? B.sparseValues.at(linearToMulti(idxB, B.dimensions))
                             .numericValue
                       : 0.0)
                : B.denseValues[idxB].numericValue;
        sum += vA * vB;
      }
      VarInfo vi;
      vi.isString = false;
      vi.numericValue = sum;
      vi.isArray = false;
      size_t idxC = static_cast<size_t>(i) * cB + j;
      if (!C.isSparse)
        C.denseValues[idxC] = vi;
      else
        C.sparseValues[linearToMulti(idxC, C.dimensions)] = vi;
    }
  }
  return C;
}

// (4) Determinant via Gaussian elimination with partial pivoting
static double detGaussian(std::vector<std::vector<double>> M) {
  int n = static_cast<int>(M.size());
  double det = 1.0;
  for (int i = 0; i < n; ++i) {
    // pivot
    int piv = i;
    for (int r = i + 1; r < n; ++r) {
      if (std::fabs(M[r][i]) > std::fabs(M[piv][i]))
        piv = r;
    }
    if (i != piv) {
      std::swap(M[i], M[piv]);
      det = -det;
    }
    double d = M[i][i];
    if (d == 0.0)
      return 0.0;
    det *= d;
    for (int r = i + 1; r < n; ++r) {
      double f = M[r][i] / d;
      for (int c = i; c < n; ++c)
        M[r][c] -= f * M[i][c];
    }
  }
  return det;
}

// Wrapper for MatrixValue

double matDeterminant(const MatrixValue &A) {
  if (A.dimensions.size() != 2 || A.dimensions[0] != A.dimensions[1])
    throw std::runtime_error("DETERMINANT requires a square 2D matrix");
  int n = A.dimensions[0];
  // build dense double matrix
  std::vector<std::vector<double>> M(n, std::vector<double>(n, 0.0));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      size_t idx = static_cast<size_t>(i) * n + j;
      if (!A.isSparse) {
        M[i][j] = A.denseValues[idx].numericValue;
      } else {
        MatrixIndex mi = linearToMulti(idx, A.dimensions);
        auto it = A.sparseValues.find(mi);
        M[i][j] = (it != A.sparseValues.end() ? it->second.numericValue : 0.0);
      }
    }
  }
  return detGaussian(M);
}

// Helper to compute linear index from multi-dimensional indices
static size_t multiToLinear(const std::vector<int> &idx,
                            const std::vector<int> &dims) {
  size_t lin = 0;
  for (size_t i = 0; i < dims.size(); ++i) {
    lin = lin * static_cast<size_t>(dims[i]) + static_cast<size_t>(idx[i]);
  }
  return lin;
}

// 6) Matrix power (integer exponent)
MatrixValue matPower(const MatrixValue &A, int exponent) {
  if (A.dimensions.size() != 2 || A.dimensions[0] != A.dimensions[1])
    throw std::runtime_error("RUNTIME ERROR: Power requires square matrix");
  int n = A.dimensions[0];
  // Create identity
  MatrixValue result;
  result.dimensions = A.dimensions;
  result.isSparse = false;
  result.denseValues.resize(n * n);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      VarInfo v;
      v.isString = false;
      v.numericValue = (i == j ? 1.0 : 0.0);
      result.denseValues[i * n + j] = v;
    }
  }
  if (exponent == 0)
    return result;
  if (exponent < 0)
    throw std::runtime_error("RUNTIME ERROR: Negative exponent not supported");
  // Exponentiation by squaring
  MatrixValue base = A;
  int exp = exponent;
  while (exp > 0) {
    if (exp & 1)
      result = matMultiply(result, base);
    base = matMultiply(base, base);
    exp >>= 1;
  }
  return result;
}

// 7) Sum of diagonal elements (trace)
double matTrace(const MatrixValue &A) {
  if (A.dimensions.size() != 2 || A.dimensions[0] != A.dimensions[1])
    throw std::runtime_error("RUNTIME ERROR: TRACE requires square matrix");
  int n = A.dimensions[0];
  double sum = 0.0;
  for (int i = 0; i < n; ++i) {
    size_t idx = static_cast<size_t>(i) * n + i;
    VarInfo v;
    if (!A.isSparse) {
      v = A.denseValues[idx];
    } else {
      MatrixIndex mi;
      mi.dimensions = {i, i};
      auto it = A.sparseValues.find(mi);
      v = (it != A.sparseValues.end() ? it->second : VarInfo());
    }
    sum += v.numericValue;
  }
  return sum;
}

// 8) Matrix rank via Gaussian elimination (double precision)
int matRank(const MatrixValue &A) {
  if (A.dimensions.size() != 2)
    throw std::runtime_error("RUNTIME ERROR: Rank requires 2D matrix");
  int rows = A.dimensions[0];
  int cols = A.dimensions[1];
  // Copy to working matrix
  std::vector<std::vector<double>> M(rows, std::vector<double>(cols));
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < cols; ++j) {
      size_t idx = i * cols + j;
      VarInfo v;
      if (!A.isSparse)
        v = A.denseValues[idx];
      else {
        MatrixIndex mi;
        mi.dimensions = {i, j};
        auto it = A.sparseValues.find(mi);
        v = (it != A.sparseValues.end() ? it->second : VarInfo());
      }
      M[i][j] = v.numericValue;
    }
  int rank = 0;
  int r = 0;
  for (int c = 0; c < cols && r < rows; ++c) {
    // Find pivot
    int pivot = r;
    for (int i = r; i < rows; ++i)
      if (std::fabs(M[i][c]) > std::fabs(M[pivot][c]))
        pivot = i;
    if (std::fabs(M[pivot][c]) < 1e-12)
      continue;
    // Swap
    std::swap(M[r], M[pivot]);
    // Normalize and eliminate
    double div = M[r][c];
    for (int j = c; j < cols; ++j)
      M[r][j] /= div;
    for (int i = 0; i < rows; ++i) {
      if (i != r) {
        double mult = M[i][c];
        for (int j = c; j < cols; ++j)
          M[i][j] -= mult * M[r][j];
      }
    }
    ++r;
    ++rank;
  }
  return rank;
}

// 9) Solve A X = B via Gaussian elimination
MatrixValue matSolve(const MatrixValue &A, const MatrixValue &B) {
  if (A.dimensions.size() != 2 || B.dimensions.size() != 2)
    throw std::runtime_error("RUNTIME ERROR: Solve requires 2D matrices");
  int n = A.dimensions[0], m = A.dimensions[1];
  int p = B.dimensions[1];
  if (B.dimensions[0] != n)
    throw std::runtime_error("RUNTIME ERROR: Solve dimension mismatch");
  // Build augmented matrix
  std::vector<std::vector<double>> M(n, std::vector<double>(m + p));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      size_t idx = i * m + j;
      VarInfo v;
      if (!A.isSparse)
        v = A.denseValues[idx];
      else {
        MatrixIndex mi{{i, j}};
        auto it = A.sparseValues.find(mi);
        v = (it != A.sparseValues.end() ? it->second : VarInfo());
      }
      M[i][j] = v.numericValue;
    }
    for (int j = 0; j < p; ++j) {
      size_t idx = i * p + j;
      VarInfo v;
      if (!B.isSparse)
        v = B.denseValues[idx];
      else {
        MatrixIndex mi{{i, j}};
        auto it = B.sparseValues.find(mi);
        v = (it != B.sparseValues.end() ? it->second : VarInfo());
      }
      M[i][m + j] = v.numericValue;
    }
  }
  // Gaussian elimination
  for (int c = 0, r = 0; c < m && r < n; ++c) {
    // pivot
    int pivot = r;
    for (int i = r; i < n; ++i)
      if (std::fabs(M[i][c]) > std::fabs(M[pivot][c]))
        pivot = i;
    if (std::fabs(M[pivot][c]) < 1e-12)
      continue;
    std::swap(M[r], M[pivot]);
    double div = M[r][c];
    for (int j = c; j < m + p; ++j)
      M[r][j] /= div;
    for (int i = 0; i < n; ++i) {
      if (i != r) {
        double mult = M[i][c];
        for (int j = c; j < m + p; ++j)
          M[i][j] -= mult * M[r][j];
      }
    }
    ++r;
  }
  // Extract solution X (n x p)
  MatrixValue X;
  X.dimensions = {m, p};
  X.isSparse = false;
  X.denseValues.resize(m * p);
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < p; ++j) {
      VarInfo v;
      v.isString = false;
      v.numericValue = M[i][m + j];
      X.denseValues[i * p + j] = v;
    }
  }
  return X;
}

// 10) Identity, Zeros, Ones
MatrixValue matIdentity(const std::vector<int> &dims) {
  if (dims.size() != 2 || dims[0] != dims[1])
    throw std::runtime_error("RUNTIME ERROR: IDENTITY requires square dims");
  int n = dims[0];
  MatrixValue M;
  M.dimensions = dims;
  M.isSparse = false;
  M.denseValues.resize(n * n);
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      VarInfo v;
      v.isString = false;
      v.numericValue = (i == j ? 1.0 : 0.0);
      M.denseValues[i * n + j] = v;
    }
  return M;
}
MatrixValue matZeros(const std::vector<int> &dims) {
  MatrixValue M;
  M.dimensions = dims;
  M.isSparse = false;
  M.denseValues.resize(
      std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<int>()));
  for (auto &v : M.denseValues) {
    v.isString = false;
    v.numericValue = 0.0;
  }
  return M;
}
MatrixValue matOnes(const std::vector<int> &dims) {
  MatrixValue M;
  M.dimensions = dims;
  M.isSparse = false;
  M.denseValues.resize(
      std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<int>()));
  for (auto &v : M.denseValues) {
    v.isString = false;
    v.numericValue = 1.0;
  }
  return M;
}

// LU Decomposition (Doolittle, no pivot)
std::pair<MatrixValue, MatrixValue> matLU(const MatrixValue &A) {
  if (A.dimensions.size() != 2 || A.dimensions[0] != A.dimensions[1])
    throw std::runtime_error("RUNTIME ERROR: LU requires square matrix");
  int n = A.dimensions[0];
  // initialize L=I, U=zero
  MatrixValue L, U;
  L.dimensions = A.dimensions;
  U.dimensions = A.dimensions;
  L.isSparse = false;
  U.isSparse = false;
  L.denseValues.resize(n * n);
  U.denseValues.resize(n * n);
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      VarInfo lv, uv;
      lv.isString = false;
      uv.isString = false;
      lv.numericValue = (i == j ? 1.0 : 0.0);
      uv.numericValue = 0.0;
      L.denseValues[i * n + j] = lv;
      U.denseValues[i * n + j] = uv;
    }
  // Doolittle
  for (int k = 0; k < n; ++k) {
    // U row
    for (int j = k; j < n; ++j) {
      double sum = 0;
      for (int t = 0; t < k; ++t)
        sum += L.denseValues[k * n + t].numericValue *
               U.denseValues[t * n + j].numericValue;
      double a = A.isSparse ? program.getMatrixValue(A, k, j)
                            : // assume helper that fetches element
                     A.denseValues[k * n + j].numericValue;
      U.denseValues[k * n + j].numericValue = a - sum;
    }
    // L col
    for (int i = k + 1; i < n; ++i) {
      double sum = 0;
      for (int t = 0; t < k; ++t)
        sum += L.denseValues[i * n + t].numericValue *
               U.denseValues[t * n + k].numericValue;
      double a = A.isSparse ? program.getMatrixValue(A, i, k)
                            : A.denseValues[i * n + k].numericValue;
      if (std::fabs(U.denseValues[k * n + k].numericValue) < 1e-12)
        throw std::runtime_error("RUNTIME ERROR: Zero pivot in LU");
      L.denseValues[i * n + k].numericValue =
          (a - sum) / U.denseValues[k * n + k].numericValue;
    }
  }
  return {L, U};
}
