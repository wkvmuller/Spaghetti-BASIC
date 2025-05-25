#include "program_structure.h"
#include "VarInfo.h"
#include <iostream>
#include <regex>
#include <stdexcept>

extern double matDeterminant(const MatrixValue&);
extern MatrixValue matInverse(const MatrixValue&);
extern MatrixValue matOnes(int, int);
extern MatrixValue matZeros(int, int);
extern MatrixValue matTranspose(const MatrixValue&);
extern void matLU(const MatrixValue&, MatrixValue&, MatrixValue&);
extern int matRank(const MatrixValue&);

void executeLine(const std::string& line) {
    std::smatch m;

    static const std::regex dimRe(R"(\s*DIM\s+([A-Z][A-Z0-9_]*)\((\d+),(\d+)\)\s*)", std::regex::icase);
    if (std::regex_match(line, m, dimRe)) {
        std::string name = m[1];
        int rows = std::stoi(m[2]), cols = std::stoi(m[3]);
        MatrixValue mat;
        mat.configureStorage({rows, cols});
        program.matrices[name] = mat;
        return;
    }

    static const std::regex letDetRe(R"(LET\s+([A-Z][A-Z0-9_]*)\s*=\s*DET\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, letDetRe)) {
        std::string var = m[1], mat = m[2];
        double val = matDeterminant(program.matrices.at(mat));
        VarInfo v; v.numericValue = val; v.isString = false;
        program.numericVariables[var] = v;
        return;
    }

    static const std::regex letRankRe(R"(LET\s+([A-Z][A-Z0-9_]*)\s*=\s*RANK\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, letRankRe)) {
        std::string var = m[1], mat = m[2];
        int val = matRank(program.matrices.at(mat));
        VarInfo v; v.numericValue = static_cast<double>(val); v.isString = false;
        program.numericVariables[var] = v;
        return;
    }

    static const std::regex matRankRe(R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*RANK\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, matRankRe)) {
        std::string var = m[1], mat = m[2];
        int val = matRank(program.matrices.at(mat));
        MatrixValue mv;
        mv.configureStorage({1, 1});
        VarInfo v; v.numericValue = static_cast<double>(val); v.isString = false;
        mv.set({0, 0}, v);
        program.matrices[var] = mv;
        return;
    }

    static const std::regex matTransRe(R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*TRANSPOSE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, matTransRe)) {
        std::string dst = m[1], src = m[2];
        program.matrices[dst] = matTranspose(program.matrices.at(src));
        return;
    }

    static const std::regex matLURe(R"(MAT\s+LU\s+([A-Z][A-Z0-9_]*)\s*=\s*([A-Z][A-Z0-9_]*)\s*)", std::regex::icase);
    if (std::regex_match(line, m, matLURe)) {
        std::string name = m[1], src = m[2];
        MatrixValue L, U;
        matLU(program.matrices.at(src), L, U);
        program.matrices[name + "_L"] = L;
        program.matrices[name + "_U"] = U;
        return;
    }

    static const std::regex matInvRe(R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*INVERSE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, matInvRe)) {
        std::string dst = m[1], src = m[2];
        program.matrices[dst] = matInverse(program.matrices.at(src));
        return;
    }

    static const std::regex matOnesRe(R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*ONES\s*\(\s*(\d+)\s*,\s*(\d+)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, matOnesRe)) {
        std::string name = m[1];
        int rows = std::stoi(m[2]), cols = std::stoi(m[3]);
        program.matrices[name] = matOnes(rows, cols);
        return;
    }

    static const std::regex matZerosRe(R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*ZEROS\s*\(\s*(\d+)\s*,\s*(\d+)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, matZerosRe)) {
        std::string name = m[1];
        int rows = std::stoi(m[2]), cols = std::stoi(m[3]);
        program.matrices[name] = matZeros(rows, cols);
        return;
    }

    throw std::runtime_error("Unknown or unhandled statement: " + line);
}