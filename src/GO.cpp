#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

//
//--------------------------------------------------------------------------------
//             prototypes
//

void evaluateMATExpression(const std::string& target, const std::string& expression);
void executeBEEP(const std::string &);
void executeBEEP(const std::string &);
void executeCLOSE(const std::string& line);
void executeDEF(const std::string &);
void executeDEF(const std::string &);}
void executeDIM(const std::string &line);
void executeFOR(const std::string &line);
void executeFORMAT(const std::string &);
void executeGO(const std::string &line);
void executeGOSUB(const std::string &line);
void executeIF(const std::string &);
void executeINPUT(const std::string &line);
void executeINPUTFILE(const std::string& line);
void executeLET(const std::string& line);
void executeMAT(const std::string& line);
void executeMATPRINT(const std::string& line);
void executeMATPRINTFILE(const std::string& line);
void executeMATREAD(const std::string& line);
void executeON(const std::string &line);
void executeOPEN(const std::string& line);
void executePRINT(const std::string& line);
void executePRINTFILE(const std::string& line);
void executePRINTFILEUSING(const std::string& line);
void executeREM(const std::string &);
void executeREM(const std::string &);}
void executeREPEAT(const std::string&);
void executeRETURN(const std::string &);
void executeSEED(const std::string& line);
void executeSTOP(const std::string &);
void executeSTOP(const std::string &);
void executeUNTIL(const std::string& line);
void executeWEND(const std::string&);
void executeWHILE(const std::string& line);
void setSparseValue(const std::string& name, const std::vector<int>& idx, const ArgsInfo& value);
void sparseMultiplyScalar(ArrayInfo& matrix, double scalar);
void sparseTrim(ArrayInfo& matrix);


//
//--------------------------------------------------------------------------------
//            Global Variables, structs, etc & helper functions.
//

enum VariableType {
  VT_UNKNOWN,
  VT_TEXT,
  VT_INT,
  VT_DOUBLE,
  VT_STRING,
  VT_CONSTANT
};

struct VarInfo {
  VariableType vT;
  std::string s;
  bool isFileOpen;
  double d;
  long long ll;
};

std::map<std::string, VarInfo> variables;

VarInfo makeVarInfo(VariableType vt, std::string tmpstr = "", double dd = 0.0,
                    long long l = 0) {
  VarInfo tmp;
  tmp.vT = vt;
  tmp.s = tmpstr;
  tmp.d = dd;
  tmp.ll = l;
  return tmp;
};

struct ArgsInfo {
  long long linenumber;
  std::string identifiername;
  bool isstring;
  std::string s;
  double d;
};

ArgsInfo makeArgsInfo(long long line, std::string idname,
                      bool boolstring = false, std::string str = "",
                      double d = 0.0) {
  ArgsInfo tmp;
  tmp.linenumber = line;
  tmp.identifiername = idname;
  tmp.isstring = boolstring;
  tmp.s = str;
  tmp.d = d;
  std::cerr << "makeArgsInfo(...)\n line<<" << line
            << " \nvar indentifyer:" << idname << "\n is string:" << boolstring
            << "\n string\"" << str << "\"\n double:" << d << std::endl;
  return tmp;
}

struct ArrayInfo {
    std::vector<int> shape;
    std::vector<double> data;                             // numeric dense
    std::map<std::vector<int>, double> sparse;            // numeric sparse
    std::vector<std::string> dataStr;                     // ← string dense
    std::map<std::vector<int>, std::string> stringSparse; // ← string sparse
};


std::map<std::string, ArrayInfo> arrays;

struct LoopFrame {
  std::string var;
  double final;
  double step;
  int returnLine;
};

enum FieldType { FIELD_TEXT, FIELD_NUMERIC, FIELD_STRING };

struct FormatField {
  FieldType type;
  std::string content;
};

std::vector<LoopFrame> loopStack;

std::stack<int> gosubStack;

struct IdentifierReturn {
  bool isstring;
  std::string s;
  double d;
};

long long currentline;  //current line we are working on.


//
//==================================================================================
//    MAT Support functions
//


void sparseTrim(ArrayInfo& matrix) {
    for (auto it = matrix.sparse.begin(); it != matrix.sparse.end(); ) {
        if (std::abs(it->second) < 1e-12) it = matrix.sparse.erase(it);
        else ++it;
    }
}

double sparseSum(const ArrayInfo& matrix) {
    double total = 0.0;
    for (const auto& [_, val] : matrix.sparse) total += val;
    return total;
}

void sparseMultiplyScalar(ArrayInfo& matrix, double scalar) {
    for (auto& [_, val] : matrix.sparse) val *= scalar;
}

ArrayInfo sparseMask(const ArrayInfo& source, const ArrayInfo& mask) {
    ArrayInfo result = source;
    result.sparse.clear();
    for (const auto& [key, val] : source.sparse) {
        if (mask.sparse.count(key) && std::abs(mask.sparse.at(key)) > 1e-12)
            result.sparse[key] = val;
    }
    return result;
}

// Helper to parse index string like "A(1,2)" into name and index vector
bool parseIndexedArray(const std::string& token, std::string& name, std::vector<int>& indices) {
    size_t open = token.find('(');
    size_t close = token.find(')');
    if (open == std::string::npos || close == std::string::npos || close < open) return false;
    name = token.substr(0, open);
    std::string indexPart = token.substr(open + 1, close - open - 1);
    std::istringstream iss(indexPart);
    std::string val;
    while (std::getline(iss, val, ',')) {
        indices.push_back(std::stoi(val));
    }
    return true;
}

// Retrieve numeric or string from sparse matrix
ArgsInfo getSparseValue(const std::string& name, const std::vector<int>& idx) {
    ArgsInfo result;
    if (arrays.count(name)) {
        if (arrays[name].sparse.count(idx)) {
            result.d = arrays[name].sparse.at(idx);
            result.isstring = false;
        } else if (arrays[name].stringSparse.count(idx)) {
            result.s = arrays[name].stringSparse.at(idx);
            result.isstring = true;
        } else {
            result.d = 0.0;
            result.s = "";
            result.isstring = false;
        }
    }
    return result;
}

// Assign value to sparse matrix
void setSparseValue(const std::string& name, const std::vector<int>& idx, const ArgsInfo& value) {
    if (!arrays.count(name)) {
        arrays[name].dimensions = idx.size();
    }
    if (value.isstring) {
        arrays[name].stringSparse[idx] = value.s;
    } else {
        arrays[name].sparse[idx] = value.d;
    }
}


bool invertMatrix(const std::vector<double>& input, std::vector<double>& output, int size) {
    output = input;
    std::vector<double> identity(size * size, 0.0);
    for (int i = 0; i < size; ++i) identity[i * size + i] = 1.0;

    for (int col = 0; col < size; ++col) {
        double diag = output[col * size + col];
        if (std::abs(diag) < 1e-12) return false;

        for (int j = 0; j < size; ++j) {
            output[col * size + j] /= diag;
            identity[col * size + j] /= diag;
        }

        for (int row = 0; row < size; ++row) {
            if (row != col) {
                double factor = output[row * size + col];
                for (int j = 0; j < size; ++j) {
                    output[row * size + j] -= factor * output[col * size + j];
                    identity[row * size + j] -= factor * identity[col * size + j];
                }
            }
        }
    }

    output = identity;
    return true;
}


double determinant(const std::vector<double>& mat, int n) {
    if (n == 1) return mat[0];
    if (n == 2) return mat[0] * mat[3] - mat[1] * mat[2];

    double det = 0.0;
    std::vector<double> submat((n - 1) * (n - 1));
    for (int col = 0; col < n; ++col) {
        int subi = 0;
        for (int i = 1; i < n; ++i) {
            int subj = 0;
            for (int j = 0; j < n; ++j) {
                if (j == col) continue;
                submat[subi * (n - 1) + subj] = mat[i * n + j];
                subj++;
            }
            subi++;
        }
        double sign = (col % 2 == 0) ? 1.0 : -1.0;
        det += sign * mat[col] * determinant(submat, n - 1);
    }
    return det;
}



void evaluateMATExpression(const std::string& target, const std::string& expression) {
    if (expr.find("DETERMINANT(") == 0) {
        size_t open = expr.find("(");
        size_t close = expr.find(")");
        std::string source = expr.substr(open + 1, close - open - 1);
        if (arrays.find(source) == arrays.end()) {
            std::cerr << "ERROR: Matrix not found: " << source << std::endl;
            return;
        }
        const ArrayInfo& mat = arrays[source];
        if (mat.dimensions != 2 || mat.shape[0] != mat.shape[1]) {
            std::cerr << "ERROR: DETERMINANT requires a square 2D matrix.";
            return;
        }
        double resultVal = determinant(mat.data, mat.shape[0]);
        ArrayInfo result;
        result.dimensions = 2;
        result.shape = {1, 1};
        result.data = { resultVal };
        arrays[target] = result;
        return;
    }

    std::string expr = expression;
    expr.erase(0, expr.find_first_not_of(" 	"));

    if (expr.find("INV(") == 0) {
        size_t open = expr.find("(");
        size_t close = expr.find(")");
        std::string source = expr.substr(open + 1, close - open - 1);
        if (arrays.find(source) == arrays.end()) {
            std::cerr << "ERROR: INV source matrix not found: " << source << std::endl;
            return;
        }

        const ArrayInfo& src = arrays[source];
        if (src.dimensions != 2 || src.shape.size() != 2 || src.shape[0] != src.shape[1]) {
            std::cerr << "ERROR: INV requires a square 2D matrix.";
            return;
        }

        ArrayInfo result;
        result.dimensions = 2;
        result.shape = src.shape;

        if (!invertMatrix(src.data, result.data, src.shape[0])) {
            std::cerr << "ERROR: INV matrix is singular or not invertible.";
            return;
        }

        arrays[target] = result;
        return;
    }

    if (expr.find("TRANS(") == 0) {
        size_t open = expr.find("(");
        size_t close = expr.find(")");
        std::string source = expr.substr(open + 1, close - open - 1);
        if (arrays.find(source) == arrays.end()) {
            std::cerr << "ERROR: TRANS source matrix not found: " << source << std::endl;
            return;
        }
        const ArrayInfo& src = arrays[source];
        if (src.dimensions != 2 || src.shape.size() != 2) {
            std::cerr << "ERROR: TRANS requires a 2D matrix." << std::endl;
            return;
        }

        ArrayInfo result;
        result.dimensions = 2;
        result.shape = { src.shape[1], src.shape[0] };
        result.data.resize(src.data.size());

        for (size_t r = 0; r < src.shape[0]; ++r) {
            for (size_t c = 0; c < src.shape[1]; ++c) {
                result.data[c * src.shape[0] + r] = src.data[r * src.shape[1] + c];
            }
        }

        arrays[target] = result;
        return;
    }

    std::istringstream iss(expr);
    std::string token1, op, token2;

    std::istringstream iss_check(expr);
    std::string left, op, right;
    iss_check >> left >> op >> right;
    if (op == "*" && arrays.find(left) == arrays.end() && arrays.find(right) != arrays.end()) {
        // SCALAR * MATRIX
        double scalar = std::stod(left);
        const ArrayInfo& mat = arrays[right];
        ArrayInfo result = mat;
        if (mat.dimensions >= 4) {
            for (auto& [key, val] : result.sparse) {
                val *= scalar;
            }
        } else {
            for (auto& val : result.data) {
                val *= scalar;
            }
        }
        arrays[target] = result;
        return;
    } else if (op == "*" && arrays.find(left) != arrays.end() && arrays.find(right) == arrays.end()) {
        // MATRIX * SCALAR
        double scalar = std::stod(right);
        const ArrayInfo& mat = arrays[left];
        ArrayInfo result = mat;
        if (mat.dimensions >= 4) {
            for (auto& [key, val] : result.sparse) {
                val *= scalar;
            }
        } else {
            for (auto& val : result.data) {
                val *= scalar;
            }
        }
        arrays[target] = result;
        return;
    }

    iss >> token1;

    if (iss >> op >> token2) {
        if (arrays.find(token1) == arrays.end() || arrays.find(token2) == arrays.end()) {
            std::cerr << "ERROR: One or both matrices not defined: " << token1 << ", " << token2 << std::endl;
            return;
        }

        const ArrayInfo& a = arrays[token1];
        const ArrayInfo& b = arrays[token2];

        if (a.dimensions != b.dimensions || a.shape != b.shape) {
            std::cerr << "ERROR: Dimension mismatch in MAT operation." << std::endl;
            return;
        }

        ArrayInfo result;
        result.dimensions = a.dimensions;
        result.shape = a.shape;

        if (a.dimensions >= 4) {
            for (const auto& entry : a.sparse) {
                if (b.sparse.find(entry.first) != b.sparse.end()) {
                    if (op == "+") {
                        result.sparse[entry.first] = entry.second + b.sparse.at(entry.first);
                    } else if (op == "-") {
                        result.sparse[entry.first] = entry.second - b.sparse.at(entry.first);
                    } else if (op == "*") {
                        result.sparse[entry.first] = entry.second * b.sparse.at(entry.first);
                    } else {
                        std::cerr << "ERROR: Unsupported operator " << op << " in sparse matrix." << std::endl;
                        return;
                    }
                }
            }
        } else {
            result.data.resize(a.data.size());
            for (size_t i = 0; i < result.data.size(); ++i) {
                if (op == "+") {
                    result.data[i] = a.data[i] + b.data[i];
                } else if (op == "-") {
                    result.data[i] = a.data[i] - b.data[i];
                } else if (op == "*") {
                    result.data[i] = a.data[i] * b.data[i];
                } else {
                    std::cerr << "ERROR: Unsupported operator " << op << " in dense matrix." << std::endl;
                    return;
                }
            }
        }

        arrays[target] = result;
    } else {
        if (arrays.find(token1) == arrays.end()) {
            std::cerr << "ERROR: Matrix not defined: " << token1 << std::endl;
            return;
        }
        arrays[target] = arrays[token1];
    }
}

//
//=======================================================================================
//   inline functsupport
//

IdentifierReturn evaluateFunction(const std::string &name,
                                  const std::vector<ArgsInfo> &args) {
  IdentifierReturn temp;

  temp.isstring = false; //  all us of temp in this routine is returning a
                         //  double - no string.

  if (name == "ASCII")
    if (!args[0].isstring || args[0].s.empty()) {
      std::cerr << "Bas string passed to ASCII(" << args[0].s
                << ")  line:" << args[0].linenumber << std::endl;
      temp.d = 0.0;
      return temp;
    } else {
      temp.d = static_cast<double>(static_cast<unsigned char>(args[0].s[0]));
      return temp;
    }

  if (name == "LEN$")
    if (!args[0].isstring) {
      std::cerr << "bad non string passed to LEN$(" << args[0].d
                << ") on line: " << args[0].linenumber << std::endl;
      temp.d = -1;
      return temp;
    } else {
      temp.d = static_cast<double>(args[0].s.length());
      return temp;
    }

  if (name == "SIN" || name == "COS" || name == "TAN" || name == "SQR" ||
          name == "STRING$" || name == "LOG" || name == "LOG10" ||
          name == "CLOG" || name == "EXP" || name == "INT" || name == "ROUND" ||
          name == "FLOOR" || name == "CEIL" | name == "RND")
    if (args[0].isstring) {
      std::cerr << "Error on " << name
                << " passing a string where number expected [" << args[0].s
                << "]  line:" << args[0].linenumber << std::endl;
      temp.d = 0.0;
      return temp;
    }

  if (name == "LOGX" || name == "POW")
    if (args[1].isstring) {
      std::cerr << "String passed [...]  " << name << "(" << args[0].d << ",["
                << args[1].s << "])  line:" << args[0].linenumber << std::endl;
      temp.d = 0.0;
      return temp;
    }

  if (name == "STRING$") {
    if (!args[0].isstring)
      temp.d = static_cast<double>(std::stoi(args[0].s));
    return temp;
  } else {
    temp.d = 0.0;
    return temp;
  }

  if (name == "LOGX") {
    temp.d = static_cast<double>(std::log(args[1].d) / std::log(args[0].d));
    return temp;
  }
  if (name == "SIN") {
    temp.d = static_cast<double>(std::log(args[1].d) / std::log(args[0].d));
    return temp;
  }
  if (name == "COS") {
    temp.d = std::cos(args[0].d);
    return temp;
  }
  if (name == "TAN") {
    temp.d = std::tan(args[0].d);
    return temp;
  }
  if (name == "SQR") {
    temp.d = std::sqrt(args[0].d);
    return temp;
  }
  if (name == "LOG") {
    temp.d = std::log(args[0].d);
    return temp;
  }
  if (name == "LOG10" || name == "CLOG") {
    temp.d = static_cast<double>(std::log10(args[0].d));
    return temp;
  }
  if (name == "EXP") {
    temp.d = std::exp(args[0].d);
    return temp;
  }
  if (name == "INT") {
    temp.d = std::floor(args[0].d);
    return temp;
  }
  if (name == "ROUND") {
    temp.d = std::round(args[0].d);
    return temp;
  }
  if (name == "FLOOR") {
    temp.d = std::floor(args[0].d);
    return temp;
  }
  if (name == "CEIL") {
    temp.d = std::ceil(args[0].d);
    return temp;
  }
  if (name == "POW") {
    temp.d = static_cast<double>(std::pow(args[0].d, args[1].d));
    return temp;
  }
  if (name == "RND") {
    temp.d = rand() / RAND_MAX;
    return temp;
  }
  std::cerr << "Unknown function: " << name << std::endl;
  temp.d = static_cast<double>(0.0);
  return temp;
}

IdentifierReturn evaluateStringFunction(const std::string &name,
                                        const std::vector<ArgsInfo> &args) {
  IdentifierReturn temp;

  temp.isstring =
      true; //  all use  of temp in this routine is returning a  string.

  if (name == "MID$" || name == "TIME$" || name == "DATE$" || name == "CHR$" ||
      name == "LEFT$" || name == "RIGHT$")
    if (!args[0].isstring) {
      std::cerr << "Passed a number [...](not a string)to " << name << "(["
                << args[0].d << "]," << args[1].d << "," << args[2].d
                << ") on line: " << args[0].linenumber << std::endl;
      temp.s = "";
      return temp;
    }
  if (name == "STRING$" || name == "CHR$")
    if (args[0].isstring) {
      std::cerr << "Passed a string [...](not a number)to " << name << "(["
                << args[0].s << "],) on line: " << args[0].linenumber
                << std::endl;
      temp.s = "";
      return temp;
    }

  if (name == "MID$" || name == "TIME$" || name == "DATE$" || name == "LEFT$" ||
      name == "RIGHT$")
    if (args[1].isstring) {
      std::cerr << "Passed a string [...](not a number)to " << name << " ("
                << args[0].s << ",[" << args[1].s
                << "],) on line: " << args[0].linenumber << std::endl;
      temp.s = "";
      return temp;
    }
  if (name == "MID$")
    if (args[2].isstring) {
      std::cerr << "Passed a string [...](not a number)to " << name << " ("
                << args[0].s << "," << args[1].d << ",[" << args[2].s
                << "]) on line: " << args[0].linenumber << std::endl;
      temp.s = "";
      return temp;
    }

  if (name == "TIME$") {
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", localtime(&now));
    temp.s = buffer;
    return temp;
  }

  if (name == "DATE$") {
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&now));
    temp.s = buffer;
    return temp;
  }

  if (name == "CHR$") {
    int c = static_cast<int>(args[0].d);
    if (c < 0 || c > 255) {
      temp.s = "";
      return temp;
    }
    temp.s = std::string(1, static_cast<char>(c));
    return temp;
  }

  if (name == "LEFT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0)
      n = 0;
    if (n > static_cast<int>(args[0].s.length()))
      n = args[0].s.length();
    temp.s = args[0].s.substr(0, n);
    return temp;
  }

  if (name == "RIGHT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0)
      n = 0;
    if (n > static_cast<int>(args[0].s.length()))
      n = args[0].s.length();
    temp.s = args[0].s.substr(args[0].s.length() - n);
    return temp;
  }
  if (name == "MID$") {
    int start = static_cast<int>(args[1].d);
    int len = static_cast<int>(args[2].d);
    if (start < 1)
      start = 1;
    if (len < 0)
      len = 0;
    if (start > static_cast<int>(args[0].s.length()))
      start = args[0].s.length();
    if (start - 1 + len > static_cast<int>(args[0].s.length()))
      len = args[0].s.length() - (start - 1);
    temp.s = args[0].s.substr(start - 1, len);
    return temp;
  }

  std::cerr << "ERROR: Unknown string function " << name << std::endl;
  temp.s = "";
  return temp;
}

// ========================= Expression Evaluator =========================

class Parser {
public:

Parser(const std::string& expr, const long long linenumber)
    : input(expr), linenumber(linenumber), pos(0) {}

private:
    std::string input;
    long long linenumber;
    size_t pos;
    std::string name;
  IdentifierReturn evalueatefunctionreturn;

  double parse() {
    double result = parseExpression();

    skipWhitespace();
    if (pos != input.length()) {
      throw std::runtime_error("Unexpected text after expression");
    }
    return result;
  }

  void skipWhitespace() {
    while (pos < input.length() && std::isspace(input[pos]))
      ++pos;
  }

  char peek() {
    skipWhitespace();
    return pos < input.length() ? input[pos] : '\0';
  }

  char get() {
    skipWhitespace();
    return pos < input.length() ? input[pos++] : '\0';
  }

  double parseExpression() {
    double value = parseTerm();
    while (true) {
      char op = peek();
      if (op == '+' || op == '-') {
        get();
        double rhs = parseTerm();
        value = (op == '+') ? value + rhs : value - rhs;
      } else
        break;
    }
    return value;
  }

  double parseTerm() {
    double value = parseFactor();
    while (true) {
      char op = peek();
      if (op == '*' || op == '/') {
        get();
        double rhs = parseFactor();
        value = (op == '*') ? value * rhs : value / rhs;
      } else
        break;
    }
    return value;
  }

  double parseFactor() {
    IdentifierReturn valreturned = parsePrimary();
    double value = valreturned.d;
    while (peek() == '^') {
      get();
      valreturned = parsePrimary();
      value = std::pow(value, valreturned.d);
    }
    return value;
  }

  std::string parseIdentifier() {
    size_t start = pos;
    while (pos < input.length() &&
           (std::isalnum(input[pos]) || input[pos] == '$'))
      ++pos;
    return input.substr(start, pos - start);
  }

  IdentifierReturn parsePrimary() {
    IdentifierReturn valreturned;
          std::vector<ArgsInfo> args;  
          skipWhitespace();
    if (peek() == '(') {
      get();
      valreturned.d = parseExpression();
      valreturned.isstring = false;
      if (get() != ')')
        throw std::runtime_error("Expected ')'");
      return valreturned;
    } else if (std::isalpha(peek())) {
      name = parseIdentifier();
      if (peek() == '(') {
        get();

        if (peek() != ')') {
          do {
            args.push_back(makeArgsInfo(linenumber , name, false, "",parseExpression()));
          } while (peek() == ',' && get());
        }
        if (get() != ')') {
          std::string errstr = "Expected ')' after function args: " +
                               std::to_string(linenumber);
          throw std::runtime_error(errstr);
        }
      }
      if (name == "TIME$" || name == "DATE$" || name == "CHR$" ||
          name == "LEFT$" || name == "RIGHT$" || name == "MID$")
        return evaluateStringFunction(name, args);
      else
        return evaluateFunction(name, args);
    } else {
      evalueatefunctionreturn.isstring = true;
      evalueatefunctionreturn.s = variables.count(name) ? variables[name] : "";
      return evalueatefunctionreturn;
    }

  } else {
    evalueatefunctionreturn.isstring = false;
    evalueatefunctionreturn.d = parseNumber();
    return evalueatefunctionreturn;
  }


double parseNumber() {
  size_t start = pos;
  while (pos < input.length() &&
         (std::isdigit(input[pos]) || input[pos] == '.'))
    ++pos;
  return std::stod(input.substr(start, pos - start));
  }
};


double evaluateExpression(const std::string &expr,
                          const long long currentline) {
  return Parser(expr, currentline).parse();
}

// ========================= Statement Handlers =========================

void executeGO(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd;
  int target;
  iss >> cmd >> target;

  if (programSource.count(target)) {
    currentLineNumber = target;
  } else {
    std::cerr << "ERROR: GO to undefined line " << target << std::endl;
    currentLineNumber = -1;
  }
}
