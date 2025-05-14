#ifndef PROGRAM_STRUCTURE_H
#define PROGRAM_STRUCTURE_H

#include <map>
#include <memory>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <utility>
#include <memory>
#include <fstream>
#include <memory>
#include <fstream>
#include <map>

const size_t DENSE_MATRIX_THRESHOLD = 10000;

struct FileHandle {
    std::unique_ptr<std::fstream> stream;
};


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
    std::map<int, std::string> programSource;
   
    // Variables
    std::map<std::string, VarInfo> numericVariables;
    std::map<std::string, VarInfo> stringVariables;

    // Matrices
    std::map<std::string, MatrixValue> numericMatrices;
    std::map<std::string, MatrixValue> stringMatrices;

    // GOSUB stack
    std::vector<int> gosubStack;

    // Loop stack (FOR/NEXT, WHILE/WEND, REPEAT/UNTIL)
    std::vector<std::pair<std::string, int>> loopStack;

    // DEF FN user-defined functions
    std::map<std::string, std::string> userFunctions;

    // DATA statements
    std::vector<VarInfo> dataValues;


    // PRINT USING formats
    std::map<int, std::string> printUsingFormats;
    
// File‐I/O channels
  std::map<int, FileHandle> fileHandles;
  
      std::string filename;
    std::string filepath;
    size_t filesize_bytes = 0;
    size_t filesize_lines = 0;
    size_t nextLineNumber = 0;
    size_t nextLineNumberSet = 0;
    size_t dataPointer = 0;    
 
};

// Declare the global instance
extern PROGRAM_STRUCTURE program;

extern int currentLine;


#endif // PROGRAM_STRUCTURE_H
