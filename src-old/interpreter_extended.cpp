#include "program_structure.h"
#include "VarInfo.h"
#include <iostream>
#include <regex>
#include <stdexcept>

extern double matDeterminant(const MatrixValue&);
extern MatrixValue matInverse(const MatrixValue&);
extern MatrixValue matOnes(int, int);
extern MatrixValue matZeros(int, int);

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
        if (!program.matrices.count(mat))
            throw std::runtime_error("Matrix not declared: " + mat);
        double val = matDeterminant(program.matrices[mat]);
        VarInfo v; v.numericValue = val; v.isString = false;
        program.numericVariables[var] = v;
        return;
    }

    static const std::regex matInvRe(R"(MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*INVERSE\s*\(\s*([A-Z][A-Z0-9_]*)\s*\))", std::regex::icase);
    if (std::regex_match(line, m, matInvRe)) {
        std::string dst = m[1], src = m[2];
        if (!program.matrices.count(src))
            throw std::runtime_error("Matrix not declared: " + src);
        program.matrices[dst] = matInverse(program.matrices[src]);
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
