#ifndef PROGRAM_STRUCTURE_H
#define PROGRAM_STRUCTURE_H

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits.h>
#include <map>
#include <memory>
#include <ratio>
#include <regex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <unordered_map>
#include <utility>


const size_t DENSE_MATRIX_THRESHOLD = 10000;

static constexpr double PI = 3.141592653589793238462643383279502884;

struct ArgsInfo {
  long long linenumber;
  std::string identifiername;
  bool isstring;
  std::string s;
  double d;
};
/*
ArgsInfo makeArgsInfo(long long line, std::string idname,
                      bool boolstring = false, std::string str = "",
                      double d = 0.0) {
  ArgsInfo tmp;
  tmp.linenumber = line;
  tmp.identifiername = idname;
  tmp.isstring = boolstring;
  tmp.s = str;
  tmp.d = d;

  std::stringstream ss;

  ss << "makeArgsInfo(...)\n line<<" << line << " \nvar indentifyer:" << idname
     << "\n is string:" << boolstring << "\n string\"" << str
     << "\"\n double:" << d << std::endl;

  std::string token = ss.str();
  throw std::runtime_error(token);
  return tmp;
}
*/
/*
struct VarInfo {
  double numericValue = 0.0;
  std::string stringValue;
  bool isString = false;
  bool isArray = false;
};
*/

enum VarType { SCALAR, ARRAY, MATRIX };

struct VarInfo {
  double numericValue = 0.0;
  std::string stringValue;
  bool isString = false;
    bool isArray = false;
  VarType type = SCALAR;

  VarInfo() = default;
  VarInfo(const std::string& str) : stringValue(str), isString(true) {}
  VarInfo(double val) : numericValue(val), isString(false) {}
};

typedef std::pair<int, int> MatrixIndex;

/*enum VariableType {
  VT_UNKNOWN,
  VT_TEXT,
  VT_INT,
  VT_DOUBLE,
  VT_STRING,
  VT_CONSTANT
};

struct IdentifierReturn {
  bool isstring;
  std::string s;
  double d;
};

/*
struct MatrixIndex {
  std::vector<int> dimensions;
  bool operator<(const MatrixIndex &other) const {
    return dimensions < other.dimensions;
  }
};
*/

struct Matrix {
    int rows, cols;
    std::vector<std::vector<double>> data;

    Matrix(int r = 0, int c = 0);
    double& operator()(int r, int c);
    double operator()(int r, int c) const;
};


struct MatrixValue {
  std::map<MatrixIndex, VarInfo> sparseValues;
  std::vector<VarInfo> denseValues;
  std::vector<int> dimensions;
  size_t totalSize = 0;
  bool isSparse = false;

  void configureStorage(const std::vector<int>& dims) {
    dimensions = dims;
    totalSize = 1;
    for (int d : dims) totalSize *= d;

    if (totalSize < DENSE_MATRIX_THRESHOLD) {
      isSparse = false;
      denseValues.resize(totalSize);
    } else {
      isSparse = true;
      sparseValues.clear();
    }
  }

  size_t flattenIndex(const MatrixIndex& index) const {
    if (dimensions.size() != 2)
      throw std::runtime_error("Only 2D matrices supported in flattenIndex()");
    return index.first * dimensions[1] + index.second;
  }

  VarInfo get(const MatrixIndex& idx) const {
    if (isSparse) {
      auto it = sparseValues.find(idx);
      if (it != sparseValues.end()) return it->second;
      VarInfo fallback;
      fallback.numericValue = 0.0;
      fallback.isString = false;
      return fallback;
    } else {
      size_t flat = flattenIndex(idx);
      if (flat >= denseValues.size()) throw std::out_of_range("Index out of bounds");
      return denseValues[flat];
    }
  }

  // Convert a linear index back to (row, col) based on dimensions
  MatrixIndex unflattenIndex(size_t idx) const {
    int cols = dimensions[1];
    return {
      static_cast<int>(idx / cols),
      static_cast<int>(idx % cols)
    };
  
  void set(const MatrixIndex& idx, const VarInfo& value) {
    if (isSparse) {
      if (value.numericValue != 0.0 || value.isString)
        sparseValues[idx] = value;
      else
        sparseValues.erase(idx);
    } else {
      size_t flat = flattenIndex(idx);
      if (flat >= denseValues.size()) throw std::out_of_range("Index out of bounds");
      denseValues[flat] = value;
    }
  }
};

// Structure to hold FOR loop state
struct ForInfo {
  std::string varName; // loop variable
  double endValue;     // upper bound
  double step;         // step increment
  int forLine;         // line number of the FOR statement
};

struct UserFunction {
  std::string param; // e.g. "X"
  std::string expr;  // e.g. "SIN(X)+10"
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
  int seedValue = 0;

  std::map<std::string, VarInfo> numericVariables;
  std::map<std::string, VarInfo> stringVariables;

  std::map<std::string, MatrixValue> matrices;
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
  
  std::vector<int> repeatStack;
};

struct pair_hash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

typedef std::unordered_map<std::pair<int, int>, double, pair_hash> SparseMatrix;

extern PROGRAM_STRUCTURE program;

extern double evalExpression(const std::string &expr);

extern std::string trim(const std::string &s);

extern std::string evalStringExpression(const std::string &expr);

#endif // PROGRAM_STRUCTURE_H
