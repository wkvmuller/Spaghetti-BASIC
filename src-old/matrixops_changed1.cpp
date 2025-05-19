#include "program_structure.h"
#include <vector>
#include <regex>
#include <stdexcept>
#include <cmath>
#include <string>

// --- Helper prototypes ---
MatrixValue matElementWiseOp(const MatrixValue &A, const MatrixValue &B, char op);
MatrixValue matScalarOp(const MatrixValue &A, double scalar, char op, bool scalarFirst = false);
double       matDeterminant(const MatrixValue &A);
MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B);
MatrixValue matPower(const MatrixValue &A, int exponent);
double       matTrace(const MatrixValue &A);
int          matRank(const MatrixValue &A);
void         matLU(const MatrixValue &A, MatrixValue &L, MatrixValue &U);

extern PROGRAM_STRUCTURE program;

// --- Local Matrix Access Helpers ---
static double getVal(const MatrixValue &M, size_t idx) {
    if (!M.isSparse) return M.denseValues[idx].numericValue;
    // compute multi-index for sparse lookup
    int D = static_cast<int>(M.dimensions.size());
    std::vector<int> coord(D);
    size_t tmp = idx;
    for (int d = D-1; d >= 0; --d) {
        coord[d] = static_cast<int>(tmp % M.dimensions[d]);
        tmp /= M.dimensions[d];
    }
    MatrixIndex mi; mi.dimensions = coord;
    auto it = M.sparseValues.find(mi);
    return it != M.sparseValues.end() ? it->second.numericValue : 0.0;
}
static double getVal(const MatrixValue &M, int i, int j) {
    size_t cols = M.dimensions.size()>1 ? static_cast<size_t>(M.dimensions[1]) : 0;
    return getVal(M, static_cast<size_t>(i)*cols + j);
}

// Dispatcher for MAT operations (1-5 patched)
void executeMAT(const std::string &line) {
    static const std::regex elemRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*([-+*/])\s*([A-Z][A-Z0-9_]*|[0-9]+(?:\.[0-9]*)?)\s*$)",
        std::regex::icase
    );
    static const std::regex detRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*DETERMINANT\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
        std::regex::icase
    );
    static const std::regex multRe(
        R"(^\s*MAT\s+MULT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\*\s*([A-Z][A-Z0-9_]*)\s*$)",
        std::regex::icase
    );
    static const std::regex powRe(
        R"(^\s*MAT\s+POWER\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\^\s*([0-9]+)\s*$)",
        std::regex::icase
    );
    static const std::regex traceRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*TRACE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
        std::regex::icase
    );
    static const std::regex rankRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*RANK\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
        std::regex::icase
    );
    static const std::regex luRe(
        R"(^\s*MAT\s+LU\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*$)",
        std::regex::icase
    );

    std::smatch m;
    if (std::regex_match(line, m, elemRe)) {
        std::string X = m[1].str();
        std::string A = m[2].str();
        char op = m[3].str()[0];
        std::string B = m[4].str();
        bool Bscalar = std::isdigit(B[0]);
        if (Bscalar) {
            double s = std::stod(B);
            program.numericMatrices[X] = matScalarOp(program.numericMatrices[A], s, op);
        } else if (std::isdigit(A[0])) {
            double s = std::stod(A);
            program.numericMatrices[m[1].str()] = matScalarOp(program.numericMatrices[B], s, op, true);
        } else {
            program.numericMatrices[X] = matElementWiseOp(
                program.numericMatrices[A],
                program.numericMatrices[B],
                op
            );
        }
    }
    else if (std::regex_match(line, m, detRe)) {
        VarInfo v;
        v.numericValue = matDeterminant(program.numericMatrices[m[2].str()]);
        v.isString = false;
        program.numericVariables[m[1].str()] = v;
    }
    else if (std::regex_match(line, m, multRe)) {
        program.numericMatrices[m[1].str()] =
            matMultiply(program.numericMatrices[m[2].str()],
                        program.numericMatrices[m[3].str()]);
    }
    else if (std::regex_match(line, m, powRe)) {
        int exp = std::stoi(m[3].str());
        program.numericMatrices[m[1].str()] =
            matPower(program.numericMatrices[m[2].str()], exp);
    }
    else if (std::regex_match(line, m, traceRe)) {
        VarInfo v;
        v.numericValue = matTrace(program.numericMatrices[m[2].str()]);
        v.isString = false;
        program.numericVariables[m[1].str()] = v;
    }
    else if (std::regex_match(line, m, rankRe)) {
        VarInfo v;
        v.numericValue = static_cast<double>(matRank(program.numericMatrices[m[2].str()]));
        v.isString = false;
        program.numericVariables[m[1].str()] = v;
    }
    else if (std::regex_match(line, m, luRe)) {
        MatrixValue L, U;
        matLU(program.numericMatrices[m[2].str()], L, U);
        program.numericMatrices[m[1].str() + "_L"] = L;
        program.numericMatrices[m[1].str() + "_U"] = U;
    }
    else {
        throw std::runtime_error("SYNTAX ERROR: Invalid MAT statement: " + line);
    }
}

// --- Helpers (1-5) implementation ---

// Element-wise binary op
MatrixValue matElementWiseOp(const MatrixValue &A, const MatrixValue &B, char op) {
    if (A.dimensions != B.dimensions)
        throw std::runtime_error("MAT ERROR: Dimension mismatch for element-wise op");
    size_t total = 1;
    for (int d : A.dimensions) total *= static_cast<size_t>(d);
    MatrixValue R = A;
    R.configureStorage(total);
    for (size_t i = 0; i < total; ++i) {
        double a = A.isSparse ? program.getMatrixValue(A, i) : A.denseValues[i].numericValue;
        double b = B.isSparse ? program.getMatrixValue(B, i) : B.denseValues[i].numericValue;
        double res;
        switch (op) {
            case '+': res = a + b; break;
            case '-': res = a - b; break;
            case '*': res = a * b; break;
            case '/': res = b == 0 ? throw std::runtime_error("MAT ERROR: Divide by zero") : a / b; break;
            default: throw std::runtime_error("MAT ERROR: Unknown op");
        }
        R.denseValues[i].numericValue = res;
    }
    return R;
}

// Scalar-matrix op
MatrixValue matScalarOp(const MatrixValue &A, double scalar, char op, bool scalarFirst) {
    size_t total = 1;
    for (int d : A.dimensions) total *= static_cast<size_t>(d);
    MatrixValue R = A;
    R.configureStorage(total);
    for (size_t i = 0; i < total; ++i) {
        double a = A.isSparse ? program.getMatrixValue(A, i) : A.denseValues[i].numericValue;
        double res;
        if (scalarFirst) {
            switch (op) {
                case '+': res = scalar + a; break;
                case '-': res = scalar - a; break;
                case '*': res = scalar * a; break;
                case '/': res = a == 0 ? throw std::runtime_error("MAT ERROR: Divide by zero") : scalar / a; break;
                default: throw std::runtime_error("MAT ERROR: Unknown op");
            }
        } else {
            switch (op) {
                case '+': res = a + scalar; break;
                case '-': res = a - scalar; break;
                case '*': res = a * scalar; break;
                case '/': res = scalar == 0 ? throw std::runtime_error("MAT ERROR: Divide by zero") : a / scalar; break;
                default: throw std::runtime_error("MAT ERROR: Unknown op");
            }
        }
        R.denseValues[i].numericValue = res;
    }
    return R;
}

// Matrix multiply
MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B) {
    if (A.dimensions.size() < 2 || B.dimensions.size() < 2 ||
        A.dimensions[A.dimensions.size()-1] != B.dimensions[B.dimensions.size()-2])
    {
        throw std::runtime_error("MAT ERROR: Dimension mismatch for multiply");
    }
    int rows = A.dimensions[A.dimensions.size()-2];
    int cols = B.dimensions[B.dimensions.size()-1];
    int inner = A.dimensions[A.dimensions.size()-1];
    MatrixValue R;
    R.dimensions = {rows, cols};
    size_t total = static_cast<size_t>(rows) * cols;
    R.configureStorage(total);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double sum = 0.0;
            for (int k = 0; k < inner; ++k) {
                sum += program.getMatrixValue(A, i, k) * program.getMatrixValue(B, k, j);
            }
            R.denseValues[i*cols + j].numericValue = sum;
        }
    }
    return R;
}

// Matrix power (integer exponent)
MatrixValue matPower(const MatrixValue &A, int exponent) {
    if (exponent < 0) throw std::runtime_error("MAT ERROR: Negative exponent");
    // assume square
    int n = A.dimensions.size()>0 ? A.dimensions[0] : 0;
    MatrixValue result = /* identity of size n */;
    MatrixValue base = A;
    while (exponent) {
        if (exponent & 1) result = matMultiply(result, base);
        base = matMultiply(base, base);
        exponent >>= 1;
    }
    return result;
}

// Trace
double matTrace(const MatrixValue &A) {
    if (A.dimensions.size() < 2 || A.dimensions[0] != A.dimensions[1])
        throw std::runtime_error("MAT ERROR: Trace requires square matrix");
    double sum = 0.0;
    int n = A.dimensions[0];
    for (int i = 0; i < n; ++i) {
        sum += program.getMatrixValue(A, i, i);
    }
    return sum;
}

// LU decomposition with getVal lambda inside:
void matLU(const MatrixValue &A, MatrixValue &L, MatrixValue &U) {
    int n = A.dimensions.size()>0 ? A.dimensions[0] : 0;
    // initialize L and U here...
    // Helper lambda to get entry (i,j):
    auto getVal = [&](const MatrixValue &M, int i, int j)->double {
        size_t cols = M.dimensions.size()>1 ? static_cast<size_t>(M.dimensions[1]) : 0;
        size_t idx  = static_cast<size_t>(i)*cols + j;
        if (!M.isSparse) return M.denseValues[idx].numericValue;
        MatrixIndex mi; mi.dimensions = {i,j};
        auto it = M.sparseValues.find(mi);
        return it!=M.sparseValues.end() ? it->second.numericValue : 0.0;
    };
    for (int k = 0; k < n; ++k) {
        for (int j = k; j < n; ++j) {
            double sum = 0.0;
            for (int t = 0; t < k; ++t) {
                sum += L.denseValues[k*n + t].numericValue *
                       U.denseValues[t*n + j].numericValue;
            }
            double a = getVal(A, k, j);
            if (std::fabs(U.denseValues[k*n + k].numericValue) < 1e-12)
                throw std::runtime_error("MAT ERROR: Zero pivot in LU");
            U.denseValues[k*n + j].numericValue = a - sum;
        }
        for (int i = k+1; i < n; ++i) {
            double sum = 0.0;
            for (int t = 0; t < k; ++t) {
                sum += L.denseValues[i*n + t].numericValue *
                       U.denseValues[t*n + k].numericValue;
            }
            double a = getVal(A, i, k);
            if (std::fabs(U.denseValues[k*n + k].numericValue) < 1e-12)
                throw std::runtime_error("MAT ERROR: Zero pivot in LU");
            L.denseValues[i*n + k].numericValue =
                (a - sum) / U.denseValues[k*n + k].numericValue;
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
                MatrixIndex mi; mi.dimensions = {i,j};
                auto it = A.sparseValues.find(mi);
                M[i][j] = (it!=A.sparseValues.end() ? it->second.numericValue : 0.0);
            }
        }
    }

    double det = 1.0;
    const double tol = 1e-12;
    for (int k = 0; k < n; ++k) {
        // find pivot row
        int pivot = k;
        double best = std::fabs(M[k][k]);
        for (int i = k+1; i < n; ++i) {
            double val = std::fabs(M[i][k]);
            if (val > best) { best = val; pivot = i; }
        }
        if (best < tol) {
            return 0.0;          // singular
        }
        if (pivot != k) {
            std::swap(M[k], M[pivot]);
            det = -det;         // row swap flips sign
        }
        det *= M[k][k];
        // eliminate below
        for (int i = k+1; i < n; ++i) {
            double factor = M[i][k] / M[k][k];
            for (int j = k; j < n; ++j) {
                M[i][j] -= factor * M[k][j];
            }
        }
    }
    return det;
}

// Rank (Gaussian elimination with partial pivoting)
int matRank(const MatrixValue &A) {
    const auto &dims = A.dimensions;
    if (dims.size() < 2) return 0;
    int n = dims[0], m = dims[1];
    // build a working copy in double
    std::vector<std::vector<double>> M(n, std::vector<double>(m));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            size_t idx = i * m + j;
            if (!A.isSparse) {
                M[i][j] = A.denseValues[idx].numericValue;
            } else {
                MatrixIndex mi; mi.dimensions = {i,j};
                auto it = A.sparseValues.find(mi);
                M[i][j] = (it!=A.sparseValues.end() ? it->second.numericValue : 0.0);
            }
        }
    }

    int rank = 0;
    const double tol = 1e-12;
    for (int col = 0; col < m && rank < n; ++col) {
        // pivot search
        int pivot = rank;
        double best = std::fabs(M[rank][col]);
        for (int r = rank+1; r < n; ++r) {
            double val = std::fabs(M[r][col]);
            if (val > best) { best = val; pivot = r; }
        }
        if (best < tol) {
            continue;  // no pivot in this column
        }
        // swap rows
        std::swap(M[rank], M[pivot]);
        // normalize pivot row
        double diag = M[rank][col];
        for (int j = col; j < m; ++j) {
            M[rank][j] /= diag;
        }
        // eliminate below
        for (int r = rank+1; r < n; ++r) {
            double factor = M[r][col];
            for (int j = col; j < m; ++j) {
                M[r][j] -= factor * M[rank][j];
            }
        }
        ++rank;
    }
    return rank;
}
void executeMAT(const std::string &line) {
    static const std::regex elemRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z0-9_.]+)\s*([-+*/])\s*([A-Z0-9_.]+)\s*$)",
        std::regex::icase
    );
    static const std::regex detRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*DETERMINANT\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
        std::regex::icase
    );
    static const std::regex multRe(
        R"(^\s*MAT\s+MULT\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\*\s*([A-Z][A-Z0-9_]*)\s*$)",
        std::regex::icase
    );
    static const std::regex powRe(
        R"(^\s*MAT\s+POWER\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\^\s*([0-9]+)\s*$)",
        std::regex::icase
    );
    static const std::regex diagRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*DIAGONAL\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
        std::regex::icase
    );
    static const std::regex rankRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*RANK\s*\(\s*([A-Z][A-Z0-9_]*)\s*\)\s*$)",
        std::regex::icase
    );
    static const std::regex solveRe(
        R"(^\s*MAT\s+SOLVE\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*\\\s*([A-Z][A-Z0-9_]*)\s*$)",
        std::regex::icase
    );
    static const std::regex identRe(
        R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*IDENTITY\s*\(\s*([0-9]+)\s*\)\s*$)",
        std::regex::icase
    );

    std::smatch m;
    if (std::regex_match(line, m, elemRe)) {
        std::string X = m[1], A = m[2], B = m[4];
        char op = m[3].str()[0];
        bool AisScalar = std::isdigit(A[0]) || A.find('.') != std::string::npos;
        bool BisScalar = std::isdigit(B[0]) || B.find('.') != std::string::npos;

        if (AisScalar && !BisScalar) {
            double scalar = std::stod(A);
            program.numericMatrices[X] = executeMATOperation((op == '+') ? 1 : 2,
                program.numericMatrices[B], {}, scalar);
        } else if (!AisScalar && BisScalar) {
            double scalar = std::stod(B);
            program.numericMatrices[X] = executeMATOperation((op == '+') ? 1 : 2,
                program.numericMatrices[A], {}, scalar);
        } else {
            program.numericMatrices[X] = executeMATOperation(3,
                program.numericMatrices[A],
                program.numericMatrices[B],
                0.0);
        }
    }
    else if (std::regex_match(line, m, detRe)) {
        VarInfo v;
        v.numericValue = executeMATOperation(4, {}, program.numericMatrices[m[2]])[0][0];
        v.isString = false;
        program.numericVariables[m[1]] = v;
    }
    else if (std::regex_match(line, m, multRe)) {
        program.numericMatrices[m[1]] = executeMATOperation(5,
            program.numericMatrices[m[2]],
            program.numericMatrices[m[3]]);
    }
    else if (std::regex_match(line, m, powRe)) {
        int exponent = std::stoi(m[3]);
        program.numericMatrices[m[1]] = executeMATOperation(6,
            {}, program.numericMatrices[m[2]], 0.0, exponent);
    }
    else if (std::regex_match(line, m, diagRe)) {
        program.numericMatrices[m[1]] = executeMATOperation(7,
            {}, program.numericMatrices[m[2]]);
    }
    else if (std::regex_match(line, m, rankRe)) {
        VarInfo v;
        v.numericValue = executeMATOperation(8, {}, program.numericMatrices[m[2]])[0][0];
        v.isString = false;
        program.numericVariables[m[1]] = v;
    }
    else if (std::regex_match(line, m, solveRe)) {
        program.numericMatrices[m[1]] = executeMATOperation(9,
            program.numericMatrices[m[2]], program.numericMatrices[m[3]]);
    }
    else if (std::regex_match(line, m, identRe)) {
        int size = std::stoi(m[2]);
        program.numericMatrices[m[1]] = executeMATOperation(10, {}, {}, 0.0, size);
    }
    else {
        throw std::runtime_error("SYNTAX ERROR: Invalid MAT statement: " + line);
    }
}
