#ifndef PROGRAM_STRUCTURE_H
#define PROGRAM_STRUCTURE_H

#include <string>
#include <map>
#include <vector>
#include <stack>
#include <utility>
#include <cstddef>

const size_t DENSE_MATRIX_THRESHOLD = 10000;

struct VarInfo {
    double numericValue = 0.0;
    std::string stringValue;
    bool isString = false;
    bool isArray = false;
};

struct MatrixIndex {
    std::vector<int> dimensions;
    bool operator<(const MatrixIndex& other) const {
        return dimensions < other.dimensions;
    }
};

struct MatrixValue {
    std::map<MatrixIndex, VarInfo> sparseValues;
    std::vector<VarInfo> denseValues;
    bool isSparse = false;
    std::vector<int> dimensions;

    void configureStorage(size_t totalElements) {
        if (totalElements < DENSE_MATRIX_THRESHOLD) {
            isSparse = false;
            denseValues.resize(totalElements);
        } else {
            isSparse = true;
            sparseValues.clear();
        }
    }
};

struct PROGRAM_STRUCTURE {
    // Program source
    std::string filename;
    std::string filepath;
    alignas(16) size_t filesize_bytes = 0;
    size_t filesize_lines = 0;
    size_t nextLineNumber = 0;
    size_t nextLineNumberSet = 0;
    int currentLine = 0;
    
    alignas(16) std::map<int, std::string> programSource;
    
    // Variables
    alignas(16) std::map<std::string, VarInfo> numericVariables;
    alignas(16) std::map<std::string, VarInfo> stringVariables;

    // Matrices
    alignas(16) std::map<std::string, MatrixValue> numericMatrices;
    alignas(16) std::map<std::string, MatrixValue> stringMatrices;

    // GOSUB stack
    alignas(16) std::vector<int> gosubStack;

    // Loop stack (FOR/NEXT, WHILE/WEND, REPEAT/UNTIL)
    alignas(16) std::vector<std::pair<std::string, int>> loopStack;

    // DEF FN user-defined functions
    alignas(16) std::map<std::string, std::string> userFunctions;

    // DATA statements
    alignas(16) std::vector<VarInfo> dataValues;
    size_t dataPointer = 0;

    // PRINT USING formats
    alignas(16) std::map<int, std::string> printUsingFormats;
};

// Declare the global instance
extern PROGRAM_STRUCTURE program;

#endif // PROGRAM_STRUCTURE_H
