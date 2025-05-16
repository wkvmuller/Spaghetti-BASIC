#ifndef PROGRAM_STRUCTURE_H
#define PROGRAM_STRUCTURE_H

#include <string>
#include <map>
#include <vector>
#include <stack>
#include <utility>
#include <cstddef>
#include <fstream>
#include <memory>

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
// Structure to hold FOR loop state
struct ForInfo {
    std::string varName;  // loop variable
    double endValue;      // upper bound
    double step;          // step increment
    int forLine;          // line number of the FOR statement
};

struct UserFunction {
    std::string param;  // e.g. "X"
    std::string expr;   // e.g. "SIN(X)+10"
};
// File handle wrapper
struct FileHandle {
    std::unique_ptr<std::fstream> stream;
};

struct PROGRAM_STRUCTURE {
    std::map<int, std::string> programSource;
    std::string filename;
    std::string filepath;
    size_t filesize_bytes = 0;
    size_t filesize_lines = 0;
    size_t nextLineNumber = 0;
    size_t nextLineNumberSet = 0;
    int currentLine = 0;
    int seedvalue = 0;

    std::map<std::string, VarInfo> numericVariables;
    std::map<std::string, VarInfo> stringVariables;

    std::map<std::string, MatrixValue> numericMatrices;
    std::map<std::string, MatrixValue> stringMatrices;

    std::vector<int> gosubStack;
    
    std::vector<std::pair<std::string, int>> loopStack;
    
    std::map<std::string, UserFunction> userFunctions;

    std::vector<VarInfo> dataValues;
    size_t dataPointer = 0;

    std::map<int, std::string> printUsingFormats;

    // Added file handles map
    std::map<int, FileHandle> fileHandles;
    
    std::vector<ForInfo> forStack;
};

extern PROGRAM_STRUCTURE program;

#endif // PROGRAM_STRUCTURE_H
