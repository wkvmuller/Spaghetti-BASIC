#ifndef PROGRAM_STRUCTURE_H
#define PROGRAM_STRUCTURE_H

#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <ratio>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <climits>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <limits.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <regex>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <climits>
#include <cstdlib> // for atoi
#include <iterator>
#include <regex>
#include <sstream>


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
  
  std::vector<int> repeatStack;

};

extern PROGRAM_STRUCTURE program;

extern double evalExpression(const std::string &expr);

extern std::string trim(const std::string &s);

extern std::string evalStringExpression(const std::string &expr);

#endif // PROGRAM_STRUCTURE_H
