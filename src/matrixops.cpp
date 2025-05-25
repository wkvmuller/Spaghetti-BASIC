#include "matrixops.h"
#include "program_structure.h"
#include <regex>
#include <stdexcept>
#include <vector>

// --- Helper prototypes ---
MatrixValue matElementWiseOp(const MatrixValue &A, const MatrixValue &B,
                             char op);
MatrixValue matScalarOp(const MatrixValue &A, double scalar, char op,
                        bool scalarFirst);
double matDeterminant(const MatrixValue &A);
MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B);
MatrixValue matPower(const MatrixValue &A, int exponent);
double matTrace(const MatrixValue &A);
int matRank(const MatrixValue &A);
void matLU(const MatrixValue &A, MatrixValue &L, MatrixValue &U);

// extern std::map<int, std::string>::const_iterator findLine(int ln);
extern int evalIntExpr(const std::string &expr);
MatrixValue executeMATOperation(const std::string &);

extern void executeMATREAD(const std::string &);
extern void executeMATPRINT(const std::string &, std::ostream &);
extern void executeMATPRINTFILE(const std::string &);

MatrixValue matInverse(const MatrixValue &);
MatrixValue matMultiply(const MatrixValue &, const MatrixValue &);

extern PROGRAM_STRUCTURE program;

// Helper to find a line in programSource or throw


  // Given a linear index, return (row, col) based on dimensions
  MatrixIndex unflattenIndex(size_t idx) const {
    int cols = dimensions[1];
    return { 
      static_cast<int>(idx / cols), 
      static_cast<int>(idx % cols) 
    };
  }

static std::map<int, std::string>::const_iterator findLine(int ln) {
  auto it = program.programSource.find(ln);
  if (it == program.programSource.end())
    throw std::runtime_error("RUNTIME ERROR: Undefined line " +
                             std::to_string(ln));
  return it;
}


double getMatrixValue(const MatrixValue &mat, int i, int j) {
  VarInfo v = mat.get({i, j});
  return v.numericValue;
}

MatrixValue matScalarOp(const MatrixValue &A, double s, char op,
                        bool scalarLeft = false) {
  MatrixValue R;
  R.configureStorage(A.dimensions);
  int rows = A.dimensions[0], cols = A.dimensions[1];

  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < cols; ++j) {
      double a = getMatrixValue(A, i, j);
      double result = 0.0;
      switch (op) {
      case '+':
        result = scalarLeft ? s + a : a + s;
        break;
      case '-':
        result = scalarLeft ? s - a : a - s;
        break;
      case '*':
        result = scalarLeft ? s * a : a * s;
        break;
      case '/':
        result = scalarLeft ? ((a == 0.0) ? 0.0 : s / a)
                            : ((s == 0.0) ? 0.0 : a / s);
        break;
      default:
        throw std::runtime_error("Invalid scalar operator");
      }
      VarInfo v;
      v.numericValue = result;
      R.set({i, j}, v);
    }

  return R;
}

MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B) {
  if (A.dimensions.size() != 2 || B.dimensions.size() != 2)
    throw std::runtime_error("Matrix multiplication requires 2D matrices");
  int aRows = A.dimensions[0], aCols = A.dimensions[1];
  int bRows = B.dimensions[0], bCols = B.dimensions[1];
  if (aCols != bRows)
    throw std::runtime_error(
        "Inner dimensions do not match for multiplication");

  MatrixValue R;
  R.configureStorage({aRows, bCols});

  for (int i = 0; i < aRows; ++i)
    for (int j = 0; j < bCols; ++j) {
      double sum = 0.0;
      for (int k = 0; k < aCols; ++k)
        sum += getMatrixValue(A, i, k) * getMatrixValue(B, k, j);
      VarInfo v;
      v.numericValue = sum;
      R.set({i, j}, v);
    }

  return R;
}

MatrixValue matPower(const MatrixValue &A, int exp) {
  if (A.dimensions.size() != 2 || A.dimensions[0] != A.dimensions[1])
    throw std::runtime_error("Matrix power requires a square matrix");

  int n = A.dimensions[0];
  MatrixValue result;
  result.configureStorage({n, n});
  // Identity matrix
  for (int i = 0; i < n; ++i) {
    VarInfo v;
    v.numericValue = 1.0;
    result.set({i, i}, v);
  }

  MatrixValue base = A;
  while (exp > 0) {
    if (exp % 2 == 1)
      result = matMultiply(result, base);
    base = matMultiply(base, base);
    exp /= 2;
  }

  return result;
}

double matTrace(const MatrixValue &A) {
  if (A.dimensions.size() != 2 || A.dimensions[0] != A.dimensions[1])
    throw std::runtime_error("Trace requires a square matrix");

  int n = A.dimensions[0];
  double sum = 0.0;
  for (int i = 0; i < n; ++i)
    sum += getMatrixValue(A, i, i);
  return sum;
}
// Helper to evaluate a BASIC expression to an int
extern int evalIntExpression(const std::string &expr);

void executeDIM(const std::string &line) {
    // Expect: DIM <name>(<expr1>,<expr2>)
    static const std::regex dimRe(R"(^\s*DIM\s+([A-Z][A-Z0-9_]*)\s*\(\s*(.+?)\s*,\s*(.+?)\s*\)\s*$)",
                                  std::regex::icase);
    std::smatch m;
    if (!std::regex_match(line, m, dimRe)) {
        throw std::runtime_error("SYNTAX ERROR in DIM: " + line);
    }

    std::string name = m[1];
    std::string exprRows = m[2];
    std::string exprCols = m[3];

    int rows = evalIntExpression(exprRows);
    int cols = evalIntExpression(exprCols);
    if (rows <= 0 || cols <= 0) {
        throw std::runtime_error("DIM: dimensions must be positive");
    }

    // Construct a fresh MatrixValue
    MatrixValue mat;
    mat.dimensions = { rows, cols };
    mat.configureStorage(mat.dimensions);

    // Store it in the program
    program.matrices[name] = std::move(mat);
}

// --- Local Matrix Access Helpers ---

// --- Helpers (1-5) implementation ---

// Element-wise binary op
MatrixValue matElementWiseOp(const MatrixValue &A,
                             const MatrixValue &B,
                             char op) {
    // Both A and B must share dimensions
    if (A.dimensions != B.dimensions)
        throw std::runtime_error("Element-wise op: dimension mismatch");
    MatrixValue R;
    R.dimensions = A.dimensions;
    R.configureStorage(R.dimensions);

    size_t total = static_cast<size_t>(R.dimensions[0]) * R.dimensions[1];
    for (size_t idx = 0; idx < total; ++idx) {
        // convert linear idx to (i,j)
        MatrixIndex mi = R.unflattenIndex(idx);

        VarInfo va = A.get(mi);
        VarInfo vb = B.get(mi);
        double a = va.numericValue;
        double b = vb.numericValue;

        double res = 0.0;
        switch (op) {
            case '+': res = a + b; break;
            case '-': res = a - b; break;
            case '*': res = a * b; break;
            case '/': res = a / b; break;
            default:  throw std::runtime_error("Unknown element-wise op");
        }

        VarInfo vr; vr.numericValue = res; vr.isString = false;
        R.set(mi, vr);
    }
    return R;
}

void matLU(const MatrixValue &A, MatrixValue &L, MatrixValue &U) {
  if (A.dimensions.size() != 2 || A.dimensions[0] != A.dimensions[1])
    throw std::runtime_error("LU: matrix must be square");

  int n = A.dimensions[0];
  L.configureStorage({n, n});
  U.configureStorage({n, n});

  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      L.set({i, j}, VarInfo{(i == j) ? 1.0 : 0.0});

  for (int i = 0; i < n; ++i) {
    for (int k = i; k < n; ++k) {
      double sum = 0.0;
      for (int j = 0; j < i; ++j)
        sum += L.get({i, j}).numericValue * U.get({j, k}).numericValue;
      double val = A.get({i, k}).numericValue - sum;
      U.set({i, k}, VarInfo{val});
    }

    for (int k = i + 1; k < n; ++k) {
      double sum = 0.0;
      for (int j = 0; j < i; ++j)
        sum += L.get({k, j}).numericValue * U.get({j, i}).numericValue;
      double val =
          (U.get({i, i}).numericValue == 0.0)
              ? 0.0
              : (A.get({k, i}).numericValue - sum) / U.get({i, i}).numericValue;
      L.set({k, i}, VarInfo{val});
    }
  }
}

// Determinant (Gaussian elimination with partial pivoting)
double matDeterminant(const MatrixValue &A) {
  const auto &dims = A.dimensions;
  if (dims.size() < 2 || dims[0] != dims[1]) {
    throw std::runtime_error("MAT ERROR: Determinant requires square matrix");
  }
  int n = dims[0];
  // build a working copy in double
  std::vector<std::vector<double>> M(n, std::vector<double>(n));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      size_t idx = i * n + j;
      if (!A.isSparse) {
        M[i][j] = A.denseValues[idx].numericValue;
      } else {
        MatrixIndex mi;
        mi.dimensions = {i, j};
        auto it = A.sparseValues.find(mi);
        M[i][j] = (it != A.sparseValues.end() ? it->second.numericValue : 0.0);
      }
    }
  }

  double det = 1.0;
  const double tol = 1e-12;
  for (int k = 0; k < n; ++k) {
    // find pivot row
    int pivot = k;
    double best = std::fabs(M[k][k]);
    for (int i = k + 1; i < n; ++i) {
      double val = std::fabs(M[i][k]);
      if (val > best) {
        best = val;
        pivot = i;
      }
    }
    if (best < tol) {
      return 0.0; // singular
    }
    if (pivot != k) {
      std::swap(M[k], M[pivot]);
      det = -det; // row swap flips sign
    }
    det *= M[k][k];
    // eliminate below
    for (int i = k + 1; i < n; ++i) {
      double factor = M[i][k] / M[k][k];
      for (int j = k; j < n; ++j) {
        M[i][j] -= factor * M[k][j];
      }
    }
  }
  return det;
}

int matRank(const MatrixValue &A) {
  if (A.dimensions.size() != 2)
    throw std::runtime_error("RANK: only 2D matrices supported");

  int m = A.dimensions[0], n = A.dimensions[1];
  std::vector<std::vector<double>> mat(m, std::vector<double>(n));
  for (int i = 0; i < m; ++i)
    for (int j = 0; j < n; ++j)
      mat[i][j] = getMatrixValue(A, i, j);

  int rank = n;
  for (int row = 0; row < rank; ++row) {
    if (mat[row][row]) {
      for (int col = 0; col < m; ++col) {
        if (col != row) {
          double mult = mat[col][row] / mat[row][row];
          for (int i = 0; i < rank; ++i)
            mat[col][i] -= mult * mat[row][i];
        }
      }
    } else {
      bool reduce = true;
      for (int i = row + 1; i < m; ++i) {
        if (mat[i][row]) {
          std::swap(mat[row], mat[i]);
          reduce = false;
          break;
        }
      }
      if (reduce) {
        for (int i = 0; i < m; ++i)
          mat[i][row] = mat[i][rank - 1];
        --rank;
        --row;
      }
    }
  }
  return rank;
}

void executeMAT(const std::string &line) {
  static const std::regex elemRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z0-9_.]+)\s*([-+*/])\s*([A-Z0-9_.]+)\s*$)",
      std::regex::icase);
  static const std::regex detRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*DETERMINANT\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex multRe(
      R"(^\s*MAT\s+MULT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\*\s*([A-Z][A-Z0-9_]*)\s*$)",
      std::regex::icase);
  static const std::regex powRe(
      R"(^\s*MAT\s+POWER\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\^\s*([0-9]+)\s*$)",
      std::regex::icase);
  static const std::regex diagRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*DIAGONAL\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex rankRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*RANK\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex solveRe(
      R"(^\s*MAT\s+SOLVE\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\\\s*([A-Z][A-Z0-9_]*)\s*$)",
      std::regex::icase);
  static const std::regex identRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*IDENTITY\s*\(\s*([0-9]+)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex traceRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*TRACE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex transRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*TRANSPOSE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex onesRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*ONES\s*\(\s*([0-9]+)\s*,\s*([0-9]+)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex zerosRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*ZEROS\s*\(\s*([0-9]+)\s*,\s*([0-9]+)\s*\)\s*$)",
      std::regex::icase);
  static const std::regex invRe(
      R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*INVERSE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
      std::regex::icase);

  std::smatch m;
  if (std::regex_match(line, m, elemRe)) {
    std::string X = m[1], A = m[2], B = m[4];
    char op = m[3].str()[0];
    bool AisScalar = std::isdigit(A[0]) || A.find('.') != std::string::npos;
    bool BisScalar = std::isdigit(B[0]) || B.find('.') != std::string::npos;

    if (AisScalar && !BisScalar) {
      double scalar = std::stod(A);
      program.matrices[X] = executeMATOperation(
          (op == '+') ? 1 : 2, program.matrices[B], {}, scalar);
    } else if (!AisScalar && BisScalar) {
      double scalar = std::stod(B);
      program.matrices[X] = executeMATOperation(
          (op == '+') ? 1 : 2, program.matrices[A], {}, scalar);
    } else {
      program.matrices[X] =
          executeMATOperation(3, program.matrices[A], program.matrices[B]);
    }
  } else if (std::regex_match(line, m, detRe)) {
    VarInfo v;
    v.numericValue = executeMATOperation(4, {}, program.matrices[m[2]])[0][0];
    v.isString = false;
    program.numericVariables[m[1]] = v;
  } else if (std::regex_match(line, m, multRe)) {
    program.matrices[m[1]] =
        executeMATOperation(5, program.matrices[m[2]], program.matrices[m[3]]);
  } else if (std::regex_match(line, m, powRe)) {
    int exponent = std::stoi(m[3]);
    program.matrices[m[1]] =
        executeMATOperation(6, {}, program.matrices[m[2]], 0.0, exponent);
  } else if (std::regex_match(line, m, diagRe)) {
    program.matrices[m[1]] = executeMATOperation(7, {}, program.matrices[m[2]]);
  } else if (std::regex_match(line, m, rankRe)) {
    VarInfo v;
    v.numericValue = executeMATOperation(8, {}, program.matrices[m[2]])[0][0];
    v.isString = false;
    program.numericVariables[m[1]] = v;
  } else if (std::regex_match(line, m, solveRe)) {
    program.matrices[m[1]] =
        executeMATOperation(9, program.matrices[m[2]], program.matrices[m[3]]);
  } else if (std::regex_match(line, m, identRe)) {
    int size = std::stoi(m[2]);
    program.matrices[m[1]] = executeMATOperation(10, {}, {}, 0.0, size);
  } else if (std::regex_match(line, m, traceRe)) {
    VarInfo v;
    v.numericValue = executeMATOperation(11, {}, program.matrices[m[2]])[0][0];
    v.isString = false;
    program.numericVariables[m[1]] = v;
  } else if (std::regex_match(line, m, transRe)) {
    program.matrices[m[1]] =
        executeMATOperation(12, {}, program.matrices[m[2]]);
  } else if (std::regex_match(line, m, onesRe)) {
    int rows = std::stoi(m[2]);
    int cols = std::stoi(m[3]);
    if (rows >= 4 || cols >= 4 || rows * cols >= 10000) {
      SparseMatrix sm;
      for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
          sm[{i, j}] = 1.0;
      program.sparseMatrices[m[1]] = sm;
    } else {
      program.matrices[m[1]] =
          executeMATOperation(13, {}, {}, static_cast<double>(cols), rows);
    }
  } else if (std::regex_match(line, m, zerosRe)) {
    int rows = std::stoi(m[2]);
    int cols = std::stoi(m[3]);
    if (rows >= 4 || cols >= 4 || rows * cols >= 10000) {
      SparseMatrix sm; // empty = all zero
      program.sparseMatrices[m[1]] = sm;
    } else {
      program.matrices[m[1]] =
          executeMATOperation(14, {}, {}, static_cast<double>(cols), rows);
    }
  } else if (std::regex_match(line, m, invRe)) {
    program.matrices[m[1]] =
        executeMATOperation(15, {}, program.matrices[m[2]]);
  } else {
    throw std::runtime_error("SYNTAX ERROR: Invalid MAT statement: " + line);
  }
}

/**
 * Dispatch all MAT‐related statements:
 *
 *   MAT <id> = <matexpr>             → executeMAT
 *   MAT READ <id>                     → executeMATREAD
 *   MAT PRINT #<chan>, <id1>,<id2>    → executeMATPRINTFILE
 *   MAT PRINT <id1>,<id2>,…           → executeMATPRINT
 */
void executeMATops(const std::string &line) {
  static const std::regex assignRe(R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*(.+)$)",
                                   std::regex::icase);
  static const std::regex readRe(R"(^\s*MAT\s+READ\s+([A-Z][A-Z0-9_]*)\s*$)",
                                 std::regex::icase);
  static const std::regex printFileRe(
      R"(^\s*MAT\s+PRINT\s*#\s*(\d+)\s*,\s*(.+)$)", std::regex::icase);
  static const std::regex printRe(R"(^\s*MAT\s+PRINT\s+(.+)$)",
                                  std::regex::icase);

  std::smatch m;
  if (std::regex_match(line, m, assignRe)) {
    // MAT <id> = <matexpr>
    executeMAT(line);
  } else if (std::regex_match(line, m, readRe)) {
    // MAT READ <id>
    executeMATREAD(line);
  } else if (std::regex_match(line, m, printFileRe)) {
    // MAT PRINT #<chan>, <id list>
    executeMATPRINTFILE(line);
  } else if (std::regex_match(line, m, printRe)) {
    // MAT PRINT <id list>
    executeMATPRINT(line, std::cout);
  } else {
    throw std::runtime_error("SYNTAX ERROR: Invalid MAT statement: " + line);
  }
}

MatrixValue executeMATOperation(const std::string &line) {
  std::smatch m;

  static const std::regex detRe(
      R"(LET\s+([A-Z][A-Z0-9_]*)\s*=\s*DET\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))",
      std::regex::icase);
  static const std::regex invRe(
      R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*INVERSE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))",
      std::regex::icase);
  static const std::regex transRe(
      R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*TRANSPOSE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))",
      std::regex::icase);
  static const std::regex rankRe(R"(RANK\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))",
                                 std::regex::icase);
  static const std::regex solveRe(
      R"(MAT\s+SOLVE\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\\\s*([A-Z][A-Z0-9_]*)\s*)",
      std::regex::icase);
  static const std::regex idRe(
      R"(MAT\s+IDENTITY\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);
  static const std::regex onesRe(R"(MAT\s+ONES\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))",
                                 std::regex::icase);
  static const std::regex zerosRe(
      R"(MAT\s+ZEROS\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);

  if (std::regex_match(line, m, detRe)) {
    const auto &mat = program.matrices.at(m[2]);
    MatrixValue R;
    R.configureStorage({1, 1});
    VarInfo v;
    v.numericValue = matDeterminant(mat);
    v.isString = false;
    R.set({0, 0}, v);
    return R;
  }

  if (std::regex_match(line, m, invRe)) {
    return matInverse(program.matrices.at(m[2]));
  }

  if (std::regex_match(line, m, transRe)) {
    return matTranspose(program.matrices.at(m[2]));
  }

  if (std::regex_match(line, m, rankRe)) {
    const auto &mat = program.matrices.at(m[1]);
    MatrixValue R;
    R.configureStorage({1, 1});
    VarInfo v;
    v.numericValue = matRank(mat);
    v.isString = false;
    R.set({0, 0}, v);
    return R;
  }

  if (std::regex_match(line, m, solveRe)) {
    return matSolve(program.matrices.at(m[2]), program.matrices.at(m[3]));
  }

  if (std::regex_match(line, m, idRe)) {
    const auto &mat = program.matrices.at(m[1]);
    if (mat.dimensions[0] != mat.dimensions[1])
      throw std::runtime_error("IDENTITY: Matrix must be square");
    return matIdentity(mat.dimensions[0]);
  }

  if (std::regex_match(line, m, onesRe)) {
    const auto &mat = program.matrices.at(m[1]);
    return matOnes(mat.dimensions[0], mat.dimensions[1]);
  }

  if (std::regex_match(line, m, zerosRe)) {
    const auto &mat = program.matrices.at(m[1]);
    return matZeros(mat.dimensions[0], mat.dimensions[1]);
  }
  static const std::regex luRe(
      R"(MAT\s+LU\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*)",
      std::regex::icase);
  if (std::regex_match(line, m, luRe)) {
    std::string dst = m[1];
    std::string src = m[2];

    if (!program.matrices.count(src))
      throw std::runtime_error("LU: source matrix not declared");

    MatrixValue L, U;
    matLU(program.matrices.at(src), L, U);
    program.matrices[dst + "_L"] = L;
    program.matrices[dst + "_U"] = U;

    MatrixValue dummy;
    dummy.configureStorage({0, 0});
    return dummy; // no result matrix, but required by function signature
  }

  throw std::runtime_error("Unsupported or malformed MAT line: " + line);
}
