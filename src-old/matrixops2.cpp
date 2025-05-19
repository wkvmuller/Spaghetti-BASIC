#include "program_structure.h"
#include <vector>
#include <regex>
#include <stdexcept>
#include <cmath>
#include <string>

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

// --- MAT Dispatcher (options 1-5) ---
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
            program.numericMatrices[X] = matScalarOp(program.numericMatrices[B], s, op, true);
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
    else {
        throw std::runtime_error("SYNTAX ERROR: Invalid MAT statement: " + line);
    }
}

// --- Helpers (1-5 & determinant & rank) ---
MatrixValue matElementWiseOp(const MatrixValue &A, const MatrixValue &B, char op) {
    if (A.dimensions != B.dimensions)
        throw std::runtime_error("MAT ERROR: Dimension mismatch");
    size_t total = 1;
    for (int d : A.dimensions) total *= static_cast<size_t>(d);
    MatrixValue R = A;
    R.configureStorage(total);
    for (size_t i = 0; i < total; ++i) {
        double a = getVal(A, i);
        double b = getVal(B, i);
        double res;
        switch (op) {
            case '+': res = a + b; break;
            case '-': res = a - b; break;
            case '*': res = a * b; break;
            case '/': if (b==0) throw std::runtime_error("MAT ERROR: Divide by zero"); res = a / b; break;
            default: throw std::runtime_error("MAT ERROR: Unknown op");
        }
        R.denseValues[i].numericValue = res;
    }
    return R;
}

MatrixValue matScalarOp(const MatrixValue &A, double scalar, char op, bool scalarFirst) {
    size_t total = 1;
    for (int d : A.dimensions) total *= static_cast<size_t>(d);
    MatrixValue R = A;
    R.configureStorage(total);
    for (size_t i = 0; i < total; ++i) {
        double a = getVal(A, i);
        double res;
        if (scalarFirst) {
            switch (op) {
                case '+': res = scalar + a; break;
                case '-': res = scalar - a; break;
                case '*': res = scalar * a; break;
                case '/': if (a==0) throw std::runtime_error("MAT ERROR: Divide by zero"); res = scalar / a; break;
                default: throw std::runtime_error("MAT ERROR: Unknown op");
            }
        } else {
            switch (op) {
                case '+': res = a + scalar; break;
                case '-': res = a - scalar; break;
                case '*': res = a * scalar; break;
                case '/': if (scalar==0) throw std::runtime_error("MAT ERROR: Divide by zero"); res = a / scalar; break;
                default: throw std::runtime_error("MAT ERROR: Unknown op");
            }
        }
        R.denseValues[i].numericValue = res;
    }
    return R;
}

MatrixValue matMultiply(const MatrixValue &A, const MatrixValue &B) {
    if (A.dimensions.size()<2 || B.dimensions.size()<2 ||
        A.dimensions.back() != B.dimensions[B.dimensions.size()-2])
        throw std::runtime_error("MAT ERROR: Dimension mismatch for multiply");
    int rows = A.dimensions[A.dimensions.size()-2];
    int cols = B.dimensions.back();
    int inner = A.dimensions.back();
    MatrixValue R;
    R.dimensions = {rows, cols};
    size_t total = static_cast<size_t>(rows)*cols;
    R.configureStorage(total);
    for (int i=0; i<rows; ++i) {
        for (int j=0; j<cols; ++j) {
            double sum=0;
            for (int k=0; k<inner; ++k) {
                sum += getVal(A,i,k)*getVal(B,k,j);
            }
            R.denseValues[static_cast<size_t>(i)*cols+j].numericValue = sum;
        }
    }
    return R;
}

MatrixValue matPower(const MatrixValue &A, int exponent) {
    if (exponent<0) throw std::runtime_error("MAT ERROR: Negative exponent");
    int n = A.dimensions.size()>0 ? A.dimensions[0] : 0;
    // identity
    MatrixValue result; result.dimensions={n,n};
    size_t total =static_cast<size_t>(n)*n;
    result.configureStorage(total);
    for (size_t i=0;i<total;++i) result.denseValues[i].numericValue=0.0;
    for (int i=0;i<n;++i) result.denseValues[static_cast<size_t>(i)*n + i].numericValue=1.0;
    MatrixValue base=A;
    while(exponent>0){
        if(exponent&1) result=matMultiply(result,base);
        base=matMultiply(base,base);
        exponent>>=1;
    }
    return result;
}

double matTrace(const MatrixValue &A) {
    if (A.dimensions.size()<2 || A.dimensions[0]!=A.dimensions[1])
        throw std::runtime_error("MAT ERROR: Trace requires square matrix");
    double sum=0;
    int n=A.dimensions[0];
    for(int i=0;i<n;++i) sum+=getVal(A,i,i);
    return sum;
}

double matDeterminant(const MatrixValue &A) {
    const auto &dims=A.dimensions;
    if(dims.size()<2||dims[0]!=dims[1]) throw std::runtime_error("MAT ERROR: Determinant requires square matrix");
    int n=dims[0];
    std::vector<std::vector<double>> M(n,std::vector<double>(n));
    for(int i=0;i<n;++i) for(int j=0;j<n;++j) {
        size_t idx=static_cast<size_t>(i)*n + j;
        M[i][j]= getVal(A,idx);
    }
    double det=1.0;
    const double tol=1e-12;
    for(int k=0;k<n;++k){
        int pivot=k; double best=fabs(M[k][k]);
        for(int i=k+1;i<n;++i){ double v=fabs(M[i][k]); if(v>best){best=v;pivot=i;} }
        if(best<tol) return 0.0;
        if(pivot!=k){ std::swap(M[k],M[pivot]); det=-det; }
        det*=M[k][k];
        for(int i=k+1;i<n;++i){ double factor=M[i][k]/M[k][k];
            for(int j=k;j<n;++j) M[i][j]-=factor*M[k][j]; }
    }
    return det;
}

int matRank(const MatrixValue &A) {
    const auto &dims=A.dimensions;
    if(dims.size()<2) return 0;
    int n=dims[0],m=dims[1];
    std::vector<std::vector<double>> M(n,std::vector<double>(m));
    for(int i=0;i<n;++i) for(int j=0;j<m;++j) {
        size_t idx=static_cast<size_t>(i)*m+j;
        M[i][j]= getVal(A,idx);
    }
    int rank=0; const double tol=1e-12;
    for(int col=0;col<m&&rank<n;++col){
        int pivot=rank; double best=fabs(M[rank][col]);
        for(int r=rank+1;r<n;++r){ double v=fabs(M[r][col]); if(v>best){best=v;pivot=r;} }
        if(best<tol) continue;
        std::swap(M[rank],M[pivot]);
        double diag=M[rank][col];
        for(int j=col;j<m;++j) M[rank][j]/=diag;
        for(int r=rank+1;r<n;++r){ double factor=M[r][col];
            for(int j=col;j<m;++j) M[r][j]-=factor*M[rank][j]; }
        ++rank;
    }
    return rank;
}
