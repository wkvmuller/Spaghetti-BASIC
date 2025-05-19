#include "matrixops.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

Matrix::Matrix(int r, int c) : rows(r), cols(c), data(r, std::vector<double>(c, 0.0)) {}

double& Matrix::operator()(int r, int c) {
    return data[r][c];
}

double Matrix::operator()(int r, int c) const {
    return data[r][c];
}

Matrix scalarOp(const Matrix &m, double scalar, char op) {
    Matrix result = m;
    for (auto &row : result.data) {
        for (auto &val : row) {
            switch (op) {
                case '+': val += scalar; break;
                case '-': val -= scalar; break;
                case '*': val *= scalar; break;
                case '/': val /= scalar; break;
                default: throw std::runtime_error("Invalid scalar operator");
            }
        }
    }
    return result;
}

Matrix elementWiseOp(const Matrix &a, const Matrix &b, char op) {
    if (a.rows != b.rows || a.cols != b.cols)
        throw std::runtime_error("Matrix dimension mismatch");
    Matrix result = a;
    for (int i = 0; i < a.rows; ++i) {
        for (int j = 0; j < a.cols; ++j) {
            switch (op) {
                case '+': result(i,j) += b(i,j); break;
                case '-': result(i,j) -= b(i,j); break;
                case '*': result(i,j) *= b(i,j); break;
                case '/': result(i,j) /= b(i,j); break;
                default: throw std::runtime_error("Invalid matrix operator");
            }
        }
    }
    return result;
}

Matrix power(const Matrix &m, int exponent) {
    if (m.rows != m.cols) throw std::runtime_error("Power: matrix must be square");
    Matrix result = identityMatrix(m.rows);
    Matrix base = m;
    while (exponent > 0) {
        if (exponent % 2 == 1) result = multiplyMatrices(result, base);
        base = multiplyMatrices(base, base);
        exponent /= 2;
    }
    return result;
}

Matrix diagonal(const Matrix &m) {
    int dim = std::min(m.rows, m.cols);
    Matrix result(dim, 1);
    for (int i = 0; i < dim; ++i) result(i, 0) = m(i, i);
    return result;
}

Matrix rankMatrix(const Matrix &m) {
    int r = computeRank(m);
    Matrix result(1, 1);
    result(0, 0) = r;
    return result;
}

Matrix solve(const Matrix &A, const Matrix &B) {
    return solveLinearSystem(A, B);
}

Matrix identityMatrixWrapper(int size) {
    return identityMatrix(size);
}

Matrix multiplyMatrices(const Matrix &a, const Matrix &b) {
    if (a.cols != b.rows) throw std::runtime_error("Matrix multiplication dimension mismatch");
    Matrix result(a.rows, b.cols);
    for (int i = 0; i < a.rows; ++i) {
        for (int j = 0; j < b.cols; ++j) {
            for (int k = 0; k < a.cols; ++k) {
                result(i,j) += a(i,k) * b(k,j);
            }
        }
    }
    return result;
}

Matrix determinantMatrix(const Matrix &m) {
    if (m.rows != m.cols) throw std::runtime_error("Determinant: matrix must be square");
    int n = m.rows;
    Matrix temp = m;
    double det = 1.0;

    for (int i = 0; i < n; ++i) {
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (fabs(temp(j, i)) > fabs(temp(pivot, i))) pivot = j;

        if (fabs(temp(pivot, i)) < 1e-12) return Matrix(1,1); // determinant is 0

        if (i != pivot) {
            std::swap(temp.data[i], temp.data[pivot]);
            det *= -1;
        }

        det *= temp(i, i);
        for (int j = i + 1; j < n; ++j) {
            double f = temp(j, i) / temp(i, i);
            for (int k = i; k < n; ++k)
                temp(j, k) -= f * temp(i, k);
        }
    }

    Matrix result(1, 1);
    result(0, 0) = det;
    return result;
}

int computeRank(const Matrix &m) {
    Matrix temp = m;
    int rank = 0;
    std::vector<bool> rowSelected(m.rows, false);

    for (int i = 0; i < m.cols; ++i) {
        int j;
        for (j = 0; j < m.rows; ++j) {
            if (!rowSelected[j] && fabs(temp(j, i)) > 1e-12)
                break;
        }

        if (j != m.rows) {
            ++rank;
            rowSelected[j] = true;
            for (int p = 0; p < m.rows; ++p) {
                if (p != j) {
                    double f = temp(p, i) / temp(j, i);
                    for (int q = i; q < m.cols; ++q)
                        temp(p, q) -= f * temp(j, q);
                }
            }
        }
    }
    return rank;
}

Matrix solveLinearSystem(const Matrix &A, const Matrix &B) {
    if (A.rows != A.cols || A.rows != B.rows) throw std::runtime_error("Solve: dimension mismatch");
    int n = A.rows, m = B.cols;
    Matrix aug(A.rows, A.cols + B.cols);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug(i, j) = A(i, j);
        for (int j = 0; j < m; ++j) aug(i, j + n) = B(i, j);
    }

    for (int i = 0; i < n; ++i) {
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (fabs(aug(j, i)) > fabs(aug(pivot, i))) pivot = j;

        if (fabs(aug(pivot, i)) < 1e-12) throw std::runtime_error("Solve: singular matrix");

        std::swap(aug.data[i], aug.data[pivot]);

        double f = aug(i, i);
        for (int j = 0; j < aug.cols; ++j)
            aug(i, j) /= f;

        for (int j = 0; j < n; ++j) {
            if (j != i) {
                double f2 = aug(j, i);
                for (int k = 0; k < aug.cols; ++k)
                    aug(j, k) -= f2 * aug(i, k);
            }
        }
    }

    Matrix result(n, m);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            result(i, j) = aug(i, j + n);

    return result;
}


// Dispatcher function
Matrix executeMATOperation(int opcode, const Matrix &A, const Matrix &B, double scalar = 0.0, int intArg = 0) {
    switch (opcode) {
        case 1: return scalarOp(B, scalar, '+');
        case 2: return scalarOp(B, scalar, '-');
        case 3: return elementWiseOp(A, B, '+');
        case 4: return determinantMatrix(B);
        case 5: return multiplyMatrices(A, B);
        case 6: return power(B, intArg);
        case 7: return diagonal(B);
        case 8: return rankMatrix(B);
        case 9: return solve(A, B);
        case 10: return identityMatrixWrapper(intArg);
        default: throw std::runtime_error("Unknown MAT opcode");
    }
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
