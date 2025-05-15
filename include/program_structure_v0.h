#ifndef PROGRAM_STRUCTURE_H
#define PROGRAM_STRUCTURE_H

#include <map>
#include <stack>
#include <string>
#include <utility>
#include <vector>

struct VarInfo {
  double numericValue = 0.0;
  std::string stringValue;
  bool isString = false;
  bool isArray = false;
};

struct MatrixIndex {
  std::vector<int> dimensions;
  bool operator<(const MatrixIndex &other) const {
    return dimensions < other.dimensions;
  }
};

struct MatrixValue {
  std::map<MatrixIndex, VarInfo> sparseValues;
  std::vector<VarInfo> denseValues;
  bool isSparse = false;
  std::vector<int> dimensions;
};

struct PROGRAM_STRUCTURE {
  // Program source
  std::map<int, std::string> programSource;
  std::string filename;
  std::string filepath;
  size_t filesize_bytes = 0;
  size_t filesize_lines = 0;

  // Variables
  std::map<std::string, VarInfo> variables;

  // Matrices
  std::map<std::string, MatrixValue> matrices;

  // GOSUB stack
  std::stack<int> gosubStack;

  // Loop stack (FOR/NEXT, WHILE/WEND, REPEAT/UNTIL)
  std::stack<std::pair<std::string, int>> loopStack;

  // DEF FN user-defined functions: name → expression string
  std::map<std::string, std::string> userFunctions;

  // DATA statement vector and pointer
  std::vector<VarInfo> dataValues;
  size_t dataPointer = 0;

  // PRINT USING formats: line number → format string
  std::map<int, std::string> printUsingFormats;
};

// Declare the global instance
extern PROGRAM_STRUCTURE program;

#endif // PROGRAM_STRUCTURE_H
