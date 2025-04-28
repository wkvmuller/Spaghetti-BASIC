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

// In your LoopFrame definition, add flags for string loops:
struct LoopFrame {
    std::string var;
    bool        isString;    // ← new
    char        startChar;   // ← new, for string loops
    double      final;       // numeric final or char‐code final
    double      step;        // numeric step or char‐code step (usually 1)
    int         returnLine;
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

static std::vector<ArgsInfo> dataBuffer;
static size_t               dataPointer = 0;


// Dispatcher for all PRINT‐variants:
//   plain PRINT,
//   PRINT USING <formatLine>,
//   PRINT #<channel>,
//   PRINT #<channel> USING <formatLine>
//
// items: a list of raw expressions or string literals to print
// channel: file handle number, or –1 for stdout
// usingFormatLine: program‐line number of a FORMAT definition, or –1 for no formatting
void evaluatePRINTexpression(
    const std::vector<std::string>& items,
    int channel = -1,
    int usingFormatLine = -1)
{
    std::ostringstream stmt;

    // Base command
    if (channel >= 0) {
        stmt << "PRINT #" << channel;
    } else {
        stmt << "PRINT";
    }

    // Optional USING clause
    if (usingFormatLine >= 0) {
        stmt << " USING " << usingFormatLine;
    }

    // Append comma + each item
    for (const auto& tok : items) {
        stmt << ",";
        // items may be literals like "\"HELLO\"" or expressions/vars
        stmt << tok;
    }

    // Dispatch to the correct handler
    const std::string line = stmt.str();
    if (channel >= 0) {
        if (usingFormatLine >= 0) {
            executePRINTFILEUSING(line);
        } else {
            executePRINTFILE(line);
        }
    } else {
        if (usingFormatLine >= 0) {
            executeFORMAT(line);
        } else {
            executePRINT(line);
        }
    }
}

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


# Replace stub evaluateMATExpression with full implementation
# Replace stub evaluateMATExpression with full implementation
void evaluateMATExpression(const std::string& target, const std::string& expression) {
    std::string expr = expression;
    expr.erase(0, expr.find_first_not_of(" \t"));

    if (expr.find("INV(") == 0) {
        size_t open = expr.find("(");
        size_t close = expr.find(")");
        std::string source = expr.substr(open + 1, close - open - 1);
        if (arrays.find(source) == arrays.end()) {
            std::cerr << "ERROR: INV source matrix not found: " << source << std::endl;
            return;
        }
        std::cerr << "[STUB] INV() not implemented; copying matrix for '" << source << "'\n";
        arrays[target] = arrays[source];
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


void executeMATPRINT(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, arrayName;
    iss >> cmd >> arrayName;
    std::cout << "[MAT STUB] MAT PRINT " << arrayName << std::endl;
}

void executeMATPRINTFILE(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, hash;
    int filenum;
    iss >> cmd >> hash >> filenum;
    std::string rest;
    std::getline(iss, rest);
    std::cout << "[MAT STUB] MAT PRINT #" << filenum << ", " << rest << std::endl;
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
          name == "FLOOR" || name == "CEIL" | name == "RND" || name = "DET" )
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
    if (name == "DET" && args.size() == 1) {
      // args[0].identifiername should be the matrix variable name
      const std::string& matName = args[0].identifiername;
      if (!arrays.count(matName)) {
          std::cerr << "ERROR: DET on undefined matrix " << matName
                    << " line:" << args[0].linenumber << std::endl;
          temp.d = 0.0;
          return temp;
      }
      const ArrayInfo& mat = arrays.at(matName);
      // Must be square 2D
      if (mat.shape.size() != 2 || mat.shape[0] != mat.shape[1]) {
          std::cerr << "ERROR: DET requires a square 2D matrix: " << matName
                    << std::endl;
          temp.d = 0.0;
          return temp;
      }
      int n = mat.shape[0];
      // Use dense data for determinant
      temp.d = determinant(mat.data, n);
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

void executeLET(const std::string& line) {
    std::istringstream iss(line);
    std::string keyword, target, eq;
    iss >> keyword >> target >> eq;
    std::string expr;
    std::getline(iss, expr);
    expr.erase(0, expr.find_first_not_of(" \t"));

    // Determine if this is a string assignment
    bool isStringVar = (!target.empty() && target.back() == '$');

    // Evaluate the value
    ArgsInfo value;
    if (isStringVar && expr.size() >= 2 && expr.front() == '"' && expr.back() == '"') {
        // Literal string
        value.isstring = true;
        value.s = expr.substr(1, expr.size() - 2);
    } else {
        // Numeric or string-from-function
        if (isStringVar) {
            // Use string evaluator
            value = evaluateStringFunction(expr, { makeArgsInfo(currentline, "", false, expr, 0.0) });
        } else {
            // Numeric
            value.isstring = false;
            value.d = evaluateExpression(expr, currentline);
        }
    }

    // Check for array indexing
    std::string name;
    std::vector<int> indices;
    if (parseIndexedArray(target, name, indices)) {
        // Assign into array (dense vs sparse handled by ArrayInfo contents)
        setSparseValue(name, indices, value);
    } else {
        // Scalar variable
        VarInfo info;
        if (value.isstring) {
            info.vT = VT_STRING;
            info.s = value.s;
        } else {
            info.vT = VT_DOUBLE;
            info.d = value.d;
        }
        variables[target] = info;
    }

    // Echo assignment
    if (value.isstring)
        std::cout << target << " = \"" << value.s << "\"" << std::endl;
    else
        std::cout << target << " = " << value.d << std::endl;
}


void executePRINT(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    std::string items;
    std::getline(iss, items);
    std::stringstream ss(items);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" 	"));
        token.erase(token.find_last_not_of(" 	") + 1);
        std::string name;
        std::vector<int> indices;
        if (parseIndexedArray(token, name, indices)) {
            ArgsInfo val = getSparseValue(name, indices);
            if (val.isstring) std::cout << val.s;
            else std::cout << val.d;
        } else {
            if (variables.count(token)) {
                VarInfo& v = variables[token];
                if (v.VT == VT_STRING) std::cout << v.s;
                else std::cout << v.d;
            } else {
                std::cout << "[undef:" << token << "]";
            }
        }
    }
    std::cout << std::endl;
} else {
        size_t paren = token.find('(');
        if (paren != std::string::npos && token.back() == ')') {
          std::string name = token.substr(0, paren);
          std::string index_str =
              token.substr(paren + 1, token.size() - paren - 2);
          std::stringstream idxs(index_str);
          std::string n;
          std::vector<int> indices;
          while (std::getline(idxs, n, ','))
            indices.push_back(std::stoi(n));
          if (arrays.count(name)) {
            ArrayInfo &arr = arrays[name];
            if (indices.size() != arr.shape.size()) {
              std::cerr << "[?]";
            } else if (!arr.data.empty()) {
              int flat = 0, stride = 1;
              for (int i = indices.size() - 1; i >= 0; --i) {
                flat += indices[i] * stride;
                stride *= arr.shape[i];
              }
              std::cout<< arr.data[flat];
            } else {
              std::cout<< arr.sparse[indices];
            }
          } else {
            std::cerr << "[ERR]";
          }
        } else {
          try {
            return evaluateExpression(token, currentline);
          } catch (...) {
            std::cerr << "[ERR]";
          }
        }
      }
      first = false;
    }
  }
  return std::endl;
}

void executeINPUT(const std::string &line) {
    // Parse optional prompt and variable list
    std::string rest = line.substr(5);
    std::string prompt;
    size_t semi = rest.find(';');
    if (semi != std::string::npos) {
        prompt = rest.substr(0, semi);
        rest = rest.substr(semi + 1);
    }
    // Trim prompt quotes
    if (!prompt.empty() && prompt.front() == '\"' && prompt.back() == '\"') {
        prompt = prompt.substr(1, prompt.size() - 2);
    }
    // Show prompt
    if (!prompt.empty()) std::cout << prompt << " ";
    std::cout << "? ";
    // Read user input line
    std::string inputLine;
    std::getline(std::cin, inputLine);

    // Tokenize input values
    std::vector<std::string> inputs;
    std::stringstream ssin(inputLine);
    std::string tok;
    while (std::getline(ssin, tok, ',')) {
        // trim whitespace
        tok.erase(0, tok.find_first_not_of(" \t"));
        tok.erase(tok.find_last_not_of(" \t") + 1);
        inputs.push_back(tok);
    }

    // Tokenize target variables
    std::vector<std::string> vars;
    std::stringstream ssvar(rest);
    while (std::getline(ssvar, tok, ',')) {
        tok.erase(0, tok.find_first_not_of(" \t"));
        tok.erase(tok.find_last_not_of(" \t") + 1);
        if (!tok.empty()) vars.push_back(tok);
    }

    // Assign values
    for (size_t i = 0; i < vars.size(); ++i) {
        // If not enough inputs, stop
        if (i >= inputs.size()) {
            std::cerr << "ERROR: Not enough input values." << std::endl;
            break;
        }
        const std::string &target = vars[i];
        const std::string &valstr = inputs[i];

        // Determine if string variable
        bool isStringVar = (!target.empty() && target.back() == '$");

        // Prepare ArgsInfo
        ArgsInfo value;
        if (isStringVar) {
            // literal or expression producing string
            if (valstr.size() >= 2 && valstr.front() == '\"' && valstr.back() == '\"') {
                value.isstring = true;
                value.s = valstr.substr(1, valstr.size() - 2);
            } else {
                // evaluate string function if needed
                value = evaluateStringFunction(valstr, { makeArgsInfo(currentline,"",false,"",0.0) });
            }
        } else {
            // numeric
            value.isstring = false;
            try {
                value.d = std::stod(valstr);
            } catch (...) {
                value.d = evaluateExpression(valstr, currentline);
            }
        }

        // Check for array indexing
        std::string name;
        std::vector<int> indices;
        if (parseIndexedArray(target, name, indices)) {
            setSparseValue(name, indices, value);
        } else {
            // Scalar variable
            VarInfo info;
            if (value.isstring) {
                info.vT = VT_STRING;
                info.s  = value.s;
            } else {
                info.vT = VT_DOUBLE;
                info.d  = value.d;
            }
            variables[target] = info;
        }
    }
}

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

void executeIF(const std::string& line) {
    // Parse out the condition and the “then” clause
    size_t thenPos = line.find("THEN");
    if (thenPos == std::string::npos) {
        std::cerr << "ERROR: Malformed IF—missing THEN\n";
        return;
    }
    // Extract and trim condition
    std::string cond = line.substr(2, thenPos - 2);
    cond.erase(0, cond.find_first_not_of(" \t"));
    cond.erase(cond.find_last_not_of(" \t") + 1);
    // Extract the statement to run if true
    std::string thenStmt = line.substr(thenPos + 4);
    thenStmt.erase(0, thenStmt.find_first_not_of(" \t"));

    // Evaluate condition for numeric or string
    bool isTrue = false;
    if (!cond.empty() && cond.front() == '"' && cond.back() == '"') {
        // literal string → true if not empty
        std::string s = cond.substr(1, cond.size() - 2);
        isTrue = !s.empty();
    }
    else if (!cond.empty() && cond.back() == '$') {
        // string variable → true if not empty
        auto it = variables.find(cond);
        if (it != variables.end() && it->second.vT == VT_STRING) {
            isTrue = !it->second.s.empty();
        }
    }
    else {
        // numeric expression → true if non-zero
        double v = evaluateExpression(cond, currentline);
        isTrue = (v != 0.0);
    }

    // If true, dispatch the THEN‐clause just like a line by itself
    if (isTrue) {
        std::istringstream iss(thenStmt);
        std::string kw;
        iss >> kw;
        for (auto& c : kw) c = std::toupper(c);
        StatementType st = identifyStatement(kw);
        switch (st) {
          case ST_PRINT:      executePRINT(thenStmt);      break;
          case ST_LET:        executeLET(thenStmt);        break;
          case ST_INPUT:      executeINPUT(thenStmt);      break;
          case ST_GOTO:       executeGOTO(thenStmt);       break;
          case ST_GOSUB:      executeGOSUB(thenStmt);      break;
          case ST_RETURN:     executeRETURN(thenStmt);     break;
          case ST_DIM:        executeDIM(thenStmt);        break;
          case ST_MAT:        executeMAT(thenStmt);        break;
          // … include any other statement types you support …
          default:
            std::cerr << "ERROR: Unsupported statement in IF THEN: " << kw << "\n";
        }
    }
}

// Updated executeFOR to handle numeric loops and single‐char string loops:
void executeFOR(const std::string& line) {
    if (loopStack.size() >= 15) {
        std::cerr << "ERROR: Maximum loop nesting (15) exceeded." << std::endl;
        currentLineNumber = -1;
        return;
    }

    std::istringstream iss(line);
    std::string cmd, var, eq, tokw;
    iss >> cmd >> var >> eq;

    bool isStringVar = !var.empty() && var.back() == '$';
    double startVal = 0.0, finalVal = 0.0, stepVal = 1.0;
    char   startChar = '\0', finalChar = '\0';

    if (isStringVar) {
        // Parse start/final as single‐char literals or variables
        std::string sStart, sFinal;
        iss >> sStart >> tokw >> sFinal;

        // Helper to get char code from literal or variable
        auto getChar = [&](const std::string& tok) {
            if (tok.size() >= 2 && tok.front()=='\"' && tok.back()=='\"')
                return tok[1];
            auto it = variables.find(tok);
            if (it != variables.end() && it->second.vT == VT_STRING && !it->second.s.empty())
                return it->second.s[0];
            return '\0';
        };

        startChar = getChar(sStart);
        finalChar = getChar(sFinal);
        startVal  = static_cast<double>(startChar);
        finalVal  = static_cast<double>(finalChar);
        // STEP on strings not supported; defaults to +1 char code
    } else {
        // Numeric FOR
        iss >> startVal >> tokw >> finalVal;
        std::string rest;
        std::getline(iss, rest);
        auto pos = rest.find("STEP");
        if (pos != std::string::npos) {
            std::istringstream ss2(rest.substr(pos + 4));
            ss2 >> stepVal;
        }
        // Initialize loop variable
        variables[var] = makeVarInfo(VT_DOUBLE, "", startVal);
    }

    // Push frame
    LoopFrame frame;
    frame.var        = var;
    frame.isString   = isStringVar;
    frame.startChar  = startChar;
    frame.final      = isStringVar ? finalVal : finalVal;
    frame.step       = isStringVar ? 1.0      : stepVal;
    frame.returnLine = currentLineNumber;
    loopStack.push_back(frame);

    // For string loops, initialize the variable
    if (isStringVar) {
        VarInfo info;
        info.vT = VT_STRING;
        info.s  = std::string(1, startChar);
        variables[var] = info;
    }
}

void executeDEF(const std::string &) {}

void executeDIM(const std::string &line) {
    // Parse variable name and dimension list
    std::string rest = line.substr(3);
    std::stringstream ss(rest);
    std::string varname, dims;
    if (!std::getline(ss, varname, '(')) return;
    varname.erase(0, varname.find_first_not_of(" \t"));
    varname.erase(varname.find_last_not_of(" \t") + 1);
    if (!std::getline(ss, dims, ')')) return;

    // Build shape vector and compute total elements
    std::stringstream dimstream(dims);
    std::string token;
    std::vector<int> shape;
    long long total = 1;
    while (std::getline(dimstream, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        int dim = 0;
        try {
            dim = std::stoi(token);
        } catch (...) {
            std::cerr << "ERROR: Invalid dimension value: " << token << std::endl;
            return;
        }
        if (dim <= 0) {
            std::cerr << "ERROR: Dimension must be positive: " << dim << std::endl;
            return;
        }
        shape.push_back(dim);
        total *= dim;
    }

    if (shape.size() > 11) {
        std::cerr << "ERROR: Too many dimensions (max 11)." << std::endl;
        return;
    }

    // Create and initialize ArrayInfo
    ArrayInfo arr;
    arr.shape = shape;

    bool isString = (!varname.empty() && varname.back() == '$');
    if (total < 10000) {
        if (isString) {
            arr.dataStr.assign(static_cast<size_t>(total), std::string());
        } else {
            arr.data.assign(static_cast<size_t>(total), 0.0);
        }
        arr.sparse.clear();
        arr.stringSparse.clear();
        std::cout << "DIM: allocated " << (isString ? "string" : "numeric")
                  << " dense array " << varname << " with " << total << " elements." << std::endl;
    } else {
        if (isString) {
            arr.dataStr.clear();
        } else {
            arr.data.clear();
        }
        arr.sparse.clear();
        arr.stringSparse.clear();
        std::cout << "DIM: initialized " << (isString ? "string" : "numeric")
                  << " sparse array " << varname << " capable of " << total << " elements." << std::endl;
    }

    arrays[varname] = std::move(arr);
}

void executeMATPRINTFILE(const std::string& line) {
    // Parse command: MAT PRINT#<filenum>, A, B$, C, ...
    std::istringstream iss(line);
    std::string cmd, hash;
    int filenum;
    iss >> cmd >> hash >> filenum;

    // Collect array names
    std::vector<std::string> arraysToPrint;
    std::string name;
    while (std::getline(iss, name, ',')) {
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        if (!name.empty()) arraysToPrint.push_back(name);
    }

    // Validate file
    if (!fileHandles.count(filenum) || !fileHandles[filenum].isFileOpen) {
        std::cerr << "ERROR: File #" << filenum << " not open." << std::endl;
        return;
    }
    std::ostream& out = *fileHandles[filenum].stream;

    // For each array name, print all elements row by row
    for (const auto& arrName : arraysToPrint) {
        if (!arrays.count(arrName)) {
            out << "[ERR: " << arrName << " undefined]\n";
            continue;
        }
        const ArrayInfo& mat = arrays[arrName];
        // Only support 2D for printing
        if (mat.shape.size() != 2) {
            out << "[ERR: " << arrName << " not 2D]\n";
            continue;
        }
        int rows = mat.shape[0], cols = mat.shape[1];
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                std::vector<int> idx = {r, c};
                ArgsInfo val = getSparseValue(arrName, idx);
                if (val.isstring) out << val.s;
                else               out << val.d;
                if (c < cols - 1) out << " ";
            }
            out << "\n";
        }
    }
}


void executeREM(const std::string &) {}

void executeSTOP(const std::string &) { std::exit(0); }

void executeGOSUB(const std::string &line) {
  if (gosubStack.size() >= 15) {
    std::cerr << "ERROR: GOSUB stack overflow (max 15 levels)." << std::endl;
    currentLineNumber = -1;
    return;
  }
  std::istringstream iss(line);
  std::string cmd;
  int target;
  iss >> cmd >> target;
  if (programSource.count(target)) {
    gosubStack.push(currentLineNumber);
    currentLineNumber = target;
  } else {
    std::cerr << "ERROR: GOSUB to undefined line " << target << std::endl;
    currentLineNumber = -1;
  }
}

void executeRETURN(const std::string &) {
  if (gosubStack.empty()) {
    std::cerr << "ERROR: RETURN without GOSUB" << std::endl;
    currentLineNumber = -1;
  } else {
    currentLineNumber = gosubStack.top();
    gosubStack.pop();
  }
}

void executeON(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, exprToken, mode;
  iss >> cmd >> exprToken >> mode;

  std::string targetList;
  std::getline(iss, targetList);
  targetList.erase(0, targetList.find_first_not_of(" 	"));

  int index = static_cast<int>(evaluateExpression(exprToken), currentline);
  if (index < 1) {
    std::cerr << "ERROR: ON " << mode << " index must be ≥ 1: " << index
              << std::endl;
    return;
  }

  std::stringstream targets(targetList);
  std::string token;
  std::vector<int> lineNumbers;
  while (std::getline(targets, token, ',')) {
    token.erase(0, token.find_first_not_of(" 	"));
    token.erase(token.find_last_not_of(" 	") + 1);
    try {
      lineNumbers.push_back(std::stoi(token));
    } catch (...) {
      std::cerr << "ERROR: Invalid line number in ON " << mode << std::endl;
      return;
    }
  }

  if (index > static_cast<int>(lineNumbers.size())) {
    std::cerr << "ERROR: ON " << mode << " index out of bounds: " << index
              << std::endl;
    return;
  }

  int targetLine = lineNumbers[index - 1];
  if (!programSource.count(targetLine)) {
    std::cerr << "ERROR: ON " << mode << " line " << targetLine
              << " does not exist." << std::endl;
    currentLineNumber = -1;
    return;
  }

  if (mode == "GOTO") {
    currentLineNumber = targetLine;
  } else if (mode == "GOSUB") {
    if (gosubStack.size() >= 15) {
      std::cerr << "ERROR: GOSUB stack overflow in ON GOSUB" << std::endl;
      currentLineNumber = -1;
      return;
    }
    gosubStack.push(currentLineNumber);
    currentLineNumber = targetLine;
  } else {
    std::cerr << "ERROR: Unsupported ON mode: " << mode << std::endl;
  }
}


void executeMAT(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, sub;
    iss >> cmd >> sub;  // cmd == "MAT", sub == next token

    if (sub == "READ") {
        // MAT READ X
        executeMATREAD(line);
    }
    else if (sub == "PRINT") {
        // MAT PRINT  …  could be console or file
        // Peek past “PRINT”
        iss >> std::ws;
        if (iss.peek() == '#') {
            // MAT PRINT #n, A, B…
            executeMATPRINTFILE(line);
        } else {
            // MAT PRINT A, B…
            executeMATPRINT(line);
        }
    }
    else {
        // MAT <target> = <expr>
        std::string target = sub, eq;
        iss >> eq;              // consume “=”
        std::string expr;
        std::getline(iss, expr);
        expr.erase(0, expr.find_first_not_of(" \t"));
        evaluateMATExpression(target, expr);
    }
}

void executeMATREAD(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, readWord, name;
    iss >> cmd >> readWord >> name;  // “MAT READ A”

    if (!arrays.count(name)) {
        std::cerr << "ERROR: MAT READ undefined matrix " << name << "\n";
        return;
    }
    ArrayInfo& mat = arrays[name];

    // Compute total elements
    size_t total = 1;
    for (int d : mat.shape) total *= d;

    // Temp index vector
    std::vector<int> idx(mat.shape.size());

    // Fill row-major order
    for (size_t n = 0; n < total; ++n) {
        // Convert flat n → multidimensional idx[]
        size_t rem = n;
        for (int dim = int(mat.shape.size()) - 1; dim >= 0; --dim) {
            idx[dim] = rem % mat.shape[dim];
            rem /= mat.shape[dim];
        }

        // Fetch next DATA item
        ArgsInfo v = getNextData();

        // Assign into dense or sparse, numeric or string
        if (!mat.data.empty()) {
            // Dense numeric
            mat.data[n] = v.isstring ? 0.0 : v.d;
        } else {
            // Sparse numeric
            if (!v.isstring)
                mat.sparse[idx] = v.d;
        }
        if (v.isstring) {
            // Store string regardless of dense/sparse
            mat.stringSparse[idx] = v.s;
        }
    }
}



std::string STRINGFORMAT(const std::string &s, const std::string &formatField) {
  size_t width = formatField.size();
  char align = formatField[0];
  std::string result;

  std::string clipped = s.length() > width ? s.substr(0, width) : s;

  std::cout << STRINGFORMAT(s, field.content);
  else std::cout << STRINGFORMAT(s, field.content);
  else std::cout << STRINGFORMAT(s, field.content);
  else {
    result = clipped + std::string(width - clipped.length(), ' ');
  }

  return result;
}

// Splits a format string into numeric, string, and text fields
std::vector<FormatField> parseFormatString(const std::string &fmt) {
  std::vector<FormatField> fields;
  std::string current;
  FieldType currentType = FIELD_TEXT;

  auto flush = [&]() {
    if (!current.empty()) {
      fields.push_back({currentType, current});
      current.clear();
    }
  };

  for (size_t i = 0; i < fmt.size(); ++i) {
    char c = fmt[i];

    if (c == '#' || c == '$') {
      if (currentType != FIELD_NUMERIC) {
        flush();
        currentType = FIELD_NUMERIC;
      }
      current += c;
    } else if (c == 'l' || c == 'r' || c == 'c') {
      if (currentType != FIELD_STRING) {
        flush();
        currentType = FIELD_STRING;
      }
      current += c;
    } else {
      if (currentType != FIELD_TEXT) {
        flush();
        currentType = FIELD_TEXT;
      }
      current += c;
    }
  }

  flush();
  return fields;
}

void executeFORMAT(const std::string &) {
  std::string formatString = formatDef.substr(pos + 2);
  formatString.erase(0, formatString.find_first_not_of(" 	\""));
  formatString.erase(formatString.find_last_not_of(" 	\"") + 1);

  std::vector<FormatField> fields = parseFormatString(formatString);

  std::vector<std::string> values;
  std::stringstream ss(printItems);
  std::string item;
  while (std::getline(ss, item, ',')) {
    item.erase(0, item.find_first_not_of(" 	"));
    item.erase(item.find_last_not_of(" 	") + 1);
    values.push_back(item);
  }

  size_t valIndex = 0;
  for (const auto &field : fields) {
    if (field.type == FIELD_TEXT) {
      std::cout << field.content;
    } else if (valIndex >= values.size()) {
      std::cerr << "[ERR: missing value]";
    } else {
      const std::string &expr = values[valIndex];
      if (field.type == FIELD_NUMERIC) {
        double val = evaluateExpression(expr);
        std::cout << val;
      } else if (field.type == FIELD_STRING) {
        std::string s = evaluateStringFunction("STRING$", {makeArgsInfo(expr)});
        size_t width = field.content.size();
        char align = field.content[0];
        if (align != 'l' && align != 'r' && align != 'c')
          align = 'l';
        if (s.length() > width)
          s = s.substr(0, width);
        if (align == 'l')
          std::cout << s << std::string(width - s.length(), ' ');
        else if (align == 'r')
          std::cout << std::string(width - s.length(), ' ') << s;
        else
          std::cout << STRINGFORMAT(s, field.content);
      }
      valIndex++;
    }
  }

  std::cout << std::endl;
}

void executeBEEP(const std::string &) { std::cout << std::string("\a"); }


void executeOPEN(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, filenameToken, forToken, modeToken, asToken, hashChannel;
    iss >> cmd >> filenameToken >> forToken >> modeToken >> asToken >> hashChannel;

    // Remove quotes from filename
    if (filenameToken.front() == '\"' && filenameToken.back() == '\"') {
        filenameToken = filenameToken.substr(1, filenameToken.length() - 2);
    }

    if (hashChannel.front() == '#') {
        hashChannel = hashChannel.substr(1);
    }

    int channel = std::stoi(hashChannel);
    OpenMode mode;
    std::ios::openmode openFlags;

    if (modeToken == "INPUT") {
        mode = MODE_INPUT;
        openFlags = std::ios::in;
    } else if (modeToken == "OUTPUT") {
        mode = MODE_OUTPUT;
        openFlags = std::ios::out | std::ios::trunc;
    } else if (modeToken == "APPEND") {
        mode = MODE_APPEND;
        openFlags = std::ios::out | std::ios::app;
    } else {
        std::cerr << "ERROR: Invalid file mode: " << modeToken << std::endl;
        return;
    }

    FileHandle fh;
    fh.filename = filenameToken;
    fh.channel = channel;
    fh.mode = mode;
    fh.stream.open(filenameToken, openFlags);

    if (!fh.stream.is_open()) {
        std::cerr << "ERROR: Could not open file '" << filenameToken << "'" << std::endl;
        return;
    }

    fh.currentCharPos = fh.stream.tellg();
    fh.stream.seekg(0, std::ios::end);
    fh.lastCharPos = fh.stream.tellg();
    fh.stream.seekg(fh.currentCharPos);
    fh.isFileOpen = true;

    openFiles[channel] = std::move(fh);
}


void executeCLOSE(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, hashChannel;
    iss >> cmd >> hashChannel;

    if (hashChannel.front() == '#') {
        hashChannel = hashChannel.substr(1);
    }

    int channel = std::stoi(hashChannel);

    auto it = openFiles.find(channel);
    if (it == openFiles.end()) {
        std::cerr << "ERROR: CLOSE attempted on unopened channel #" << channel << std::endl;
        return;
    }
    it->second.isFileOpen = false;

    it->second.stream.close();
    openFiles.erase(it);
}


void executePRINTFILE(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, hashChannel;
    iss >> cmd >> hashChannel;

    if (hashChannel.front() == '#') {
        hashChannel = hashChannel.substr(1);
    }

    int channel = std::stoi(hashChannel);
    auto it = openFiles.find(channel);
    if (it == openFiles.end() || !it->second.isFileOpen) {
        std::cerr << "ERROR: PRINT# attempted on unopened or closed channel #" << channel << std::endl;
        return;
    }

    std::string rest;
    std::getline(iss, rest);
    size_t comma = rest.find(',');
    if (comma != std::string::npos) {
        rest = rest.substr(comma + 1);
    }

    std::stringstream ss(rest);
    std::string token;
    bool first = true;

    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);

        auto v = variables.find(token);
        if (v != variables.end()) {
            if (!first) it->second.stream << " ";
            first = false;

            if (v->second.vT == VT_STRING || v->second.vT == VT_TEXT) {
                it->second.stream << v->second.s;
            } else if (v->second.vT == VT_INT) {
                it->second.stream << v->second.ll;
            } else {
                it->second.stream << v->second.d;
            }
        } else {
            it->second.stream << token;
        }
    }

    it->second.stream << std::endl;
}

void executeINPUTFILE(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, hashChannel;
    iss >> cmd >> hashChannel;

    if (hashChannel.front() == '#') {
        hashChannel = hashChannel.substr(1);
    }

    int channel = std::stoi(hashChannel);
    auto it = openFiles.find(channel);
    if (it == openFiles.end() || !it->second.isFileOpen) {
        std::cerr << "ERROR: INPUT# attempted on unopened or closed channel #" << channel << std::endl;
        return;
    }

    std::string rest;
    std::getline(iss, rest);
    size_t comma = rest.find(',');
    if (comma != std::string::npos) {
        rest = rest.substr(comma + 1);
    }

    std::stringstream varList(rest);
    std::string varname;
    while (std::getline(varList, varname, ',')) {
        varname.erase(0, varname.find_first_not_of(" \t"));
        varname.erase(varname.find_last_not_of(" \t") + 1);

        std::string val;
        if (!(it->second.stream >> val)) {
            std::cerr << "ERROR: Failed to read value from channel #" << channel << std::endl;
            return;
        }

        VarInfo v;
        if (!varname.empty() && varname.back() == '$') {
            v.vT = VT_STRING;
            v.s = val;
        } else {
            try {
                v.d = std::stod(val);
                v.vT = VT_DOUBLE;
            } catch (...) {
                std::cerr << "ERROR: INPUT# value '" << val << "' is not a number for variable '" << varname << "'" << std::endl;
                return;
            }
        }

        variables[varname] = v;
    }
}

void executeWHILE(const std::string& line) {
    if (loopStack.size() >= 15) {
        std::cerr << "ERROR: WHILE nesting exceeded limit (15)." << std::endl;
        currentLineNumber = -1;
        return;
    }

    std::string cond = line.substr(5);
    cond.erase(0, cond.find_first_not_of(" \t"));

    if (evaluateExpression(cond) == 0.0) {
        int depth = 1;
        auto it = programSource.upper_bound(currentLineNumber);
        while (it != programSource.end()) {
            std::string upper = it->second;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            if (upper.find("WHILE") == 0) depth++;
            else if (upper.find("WEND") == 0) {
                depth--;
                if (depth == 0) {
                    currentLineNumber = it->first;
                    return;
                }
            }
            ++it;
        }
        std::cerr << "ERROR: WHILE without matching WEND." << std::endl;
        currentLineNumber = -1;
        return;
    }

    loopStack.push_back({"WHILE", cond, currentLineNumber});
}

void executeWEND(const std::string&) {
    if (loopStack.empty() || loopStack.back().type != "WHILE") {
        std::cerr << "ERROR: WEND without matching WHILE" << std::endl;
        currentLineNumber = -1;
        return;
    }

    LoopFrame frame = loopStack.back();
    if (evaluateExpression(frame.condition) != 0.0) {
        currentLineNumber = frame.returnLine;
    } else {
        loopStack.pop_back();
    }
}

void executeREPEAT(const std::string&) {
    if (loopStack.size() >= 15) {
        std::cerr << "ERROR: REPEAT nesting exceeded limit (15)." << std::endl;
        currentLineNumber = -1;
        return;
    }

    loopStack.push_back({"REPEAT", "", currentLineNumber});
}

void executeUNTIL(const std::string& line) {
    if (loopStack.empty() || loopStack.back().type != "REPEAT") {
        std::cerr << "ERROR: UNTIL without matching REPEAT" << std::endl;
        currentLineNumber = -1;
        return;
    }

    std::string cond = line.substr(5);
    cond.erase(0, cond.find_first_not_of(" \t"));

    if (evaluateExpression(cond) == 0.0) {
        currentLineNumber = loopStack.back().returnLine;
    } else {
        loopStack.pop_back();
    }
}

void executeSEED(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    int seed;
    iss >> cmd >> seed;

    srand(seed);  // seed RNG
    std::cout << "RNG seeded with value: " << seed << std::endl;
}

void executePRINTFILEUSING(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, hashToken, usingToken;
    int filenum = -1, formatLine = -1;

    iss >> cmd >> hashToken >> filenum >> usingToken >> formatLine;

    std::string printItems;
    std::getline(iss, printItems);
    printItems.erase(0, printItems.find_first_not_of(" \t,"));

    if (!fileHandles.count(filenum) || !fileHandles[filenum].isFileOpen) {
        std::cerr << "ERROR: File #" << filenum << " is not open." << std::endl;
        return;
    }

    if (!programSource.count(formatLine)) {
        std::cerr << "ERROR: Format line " << formatLine << " not found." << std::endl;
        return;
    }

    std::string formatDef = programSource[formatLine];
    size_t pos = formatDef.find(":=");
    if (pos == std::string::npos) {
        std::cerr << "ERROR: Format line " << formatLine << " missing :=." << std::endl;
        return;
    }

    std::string formatString = formatDef.substr(pos + 2);
    formatString.erase(0, formatString.find_first_not_of(" \t\""));
    formatString.erase(formatString.find_last_not_of(" \t\"") + 1);

    std::vector<FormatField> fields = parseFormatString(formatString);

    std::vector<std::string> values;
    std::stringstream ss(printItems);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        values.push_back(item);
    }

    std::ostream& out = *(fileHandles[filenum].stream);
    size_t valIndex = 0;
    for (const auto& field : fields) {
        if (field.type == FIELD_TEXT) {
            out << field.content;
        } else if (valIndex >= values.size()) {
            out << "[ERR: missing value]";
        } else {
            const std::string& expr = values[valIndex];
            if (field.type == FIELD_NUMERIC) {
                double val = evaluateExpression(expr);
                out << val;
            } else if (field.type == FIELD_STRING) {
                ArgsInfo sval = evaluateStringFunction("STRING$", {makeArgsInfo(expr)});
                out << STRINGFORMAT(sval.s, field.content);
            }
            valIndex++;
        }
    }

    out << std::endl;
}


void executeMAT(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, target, equals;
    iss >> cmd >> target >> equals;
    std::string expression;
    std::getline(iss, expression);
    expression.erase(0, expression.find_first_not_of(" \t"));
    evaluateMATExpression(target, expression);
}

// ─────────────────────────────────────────────────────────────────────────────
// Reads DATA statements into a previously DIM’d matrix (numeric only).
// Usage in BASIC:  MAT READ A
// It will collect all DATA values in program order and load them into A.

void executeMATREAD(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, name;
    iss >> cmd >> name;  // cmd == "MAT", next token is "READ", then name?
    // Some implementations write "MAT READ A". If yours uses that form:
    if (name == "READ") {
        iss >> name;
    }

    auto it = arrays.find(name);
    if (it == arrays.end()) {
        std::cerr << "ERROR: MAT READ on undefined matrix: " << name << std::endl;
        return;
    }
    ArrayInfo &mat = it->second;

    // Compute total elements = product of dimensions
    size_t total = 1;
    for (int d : mat.shape) {
        total *= d;
    }

    // Gather all DATA values
    std::vector<double> values;
    for (const auto &p : programSource) {
        std::istringstream ds(p.second);
        std::string kw;
        ds >> kw;
        if (kw == "DATA") {
            std::string rest;
            std::getline(ds, rest);
            std::stringstream vs(rest);
            std::string token;
            while (std::getline(vs, token, ',')) {
                // trim
                token.erase(0, token.find_first_not_of(" \t"));
                token.erase(token.find_last_not_of(" \t") + 1);
                try {
                    values.push_back(std::stod(token));
                } catch (...) {
                    values.push_back(0.0);
                }
            }
        }
    }

    if (values.size() < total) {
        std::cerr << "ERROR: MAT READ, not enough DATA values ("
                  << values.size() << " available for "
                  << total << " elements)" << std::endl;
        return;
    }

    // Load into dense storage (row-major)
    mat.data.clear();
    mat.data.reserve(total);
    for (size_t i = 0; i < total; ++i) {
        mat.data.push_back(values[i]);
    }
}

void executeMATPRINT(const std::string& line) {
    // Expect line of form “MAT PRINT A, B$, C”
    std::istringstream iss(line);
    std::string matKw, printKw;
    iss >> matKw >> printKw; // “MAT” and “PRINT”

    // Collect the matrix names
    std::vector<std::string> arraysToPrint;
    std::string name;
    while (std::getline(iss, name, ',')) {
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        if (!name.empty()) arraysToPrint.push_back(name);
    }

    // For each matrix, print all elements row by row
    for (const auto& arrName : arraysToPrint) {
        if (!arrays.count(arrName)) {
            std::cout << "[ERR: " << arrName << " undefined]\n";
            continue;
        }
        const ArrayInfo& mat = arrays.at(arrName);
        if (mat.shape.size() != 2) {
            std::cout << "[ERR: " << arrName << " not 2D]\n";
            continue;
        }
        int rows = mat.shape[0], cols = mat.shape[1];
        std::cout << "Matrix " << arrName << " (" << rows << "×" << cols << "):\n";
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                std::vector<int> idx = {r, c};
                // getSparseValue handles dense vs sparse, numeric vs string
                ArgsInfo v = getSparseValue(arrName, idx);
                if (v.isstring)        std::cout << v.s;
                else                   std::cout << v.d;
                if (c < cols - 1)      std::cout << "\t";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}



// Restore DATA pointer
void executeRESTORE(const std::string&) {
    dataPointer = 0;
}

// Parse DATA statements into the buffer
void executeDATA(const std::string& line) {
    // Remove leading "DATA"
    std::string rest = line.substr(4);
    // Split on commas
    std::stringstream ss(rest);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (token.size() >= 2 && token.front() == '\"' && token.back() == '\"') {
            // String literal
            std::string content = token.substr(1, token.size() - 2);
            dataBuffer.push_back(makeArgsInfo(currentline, "", true, content, 0.0));
        } else {
            // Numeric value
            double val = 0.0;
            try {
                val = std::stod(token);
            } catch (...) {
                std::cerr << "ERROR: Invalid DATA value: " << token
                          << " on line " << currentline << std::endl;
            }
            dataBuffer.push_back(makeArgsInfo(currentline, "", false, "", val));
        }
    }
}


// ========================= Dispatcher =========================

enum StatementType {
  ST_UNKNOWN,
  ST_LET,
  ST_PRINT,
  ST_INPUT,
  ST_GOTO,
  ST_IF,
  ST_FOR,
  ST_NEXT,
  ST_READ,
  ST_DATA,
  ST_RESTORE,
  ST_END,
  ST_DEF,
  ST_DIM,
  ST_REM,
  ST_STOP,
  ST_GOSUB,
  ST_RETURN,
  ST_ON,
  ST_MAT,
  ST_FORMAT,
  ST_BEEP,
  ST_OPEN,
  ST_CLOSE,
  ST_PRINTFILE,
  ST_INPUTFILE,
  ST_WHILE,
  ST_WEND,
  ST_REPEAT,
  ST_UNTIL,
  ST_SEED,
  ST_MATREAD
};

StatementType identifyStatement(const std::string &keyword) {
  if (keyword == "LET")
    return ST_LET;
  if (keyword == "PRINT")
    return ST_PRINT;
  if (keyword == "INPUT")
    return ST_INPUT;
  if (keyword == "GOTO")
    return ST_GOTO;
  if (keyword == "IF")
    return ST_IF;
  if (keyword == "FOR")
    return ST_FOR;
  if (keyword == "NEXT")
    return ST_NEXT;
  if (keyword == "READ")
    return ST_READ;
  if (keyword == "DATA")
    return ST_DATA;
  if (keyword == "RESTORE")
    return ST_RESTORE;
  if (keyword == "END")
    return ST_END;
  if (keyword == "DEF")
    return ST_DEF;
  if (keyword == "DIM")
    return ST_DIM;
  if (keyword == "REM")
    return ST_REM;
  if (keyword == "STOP")
    return ST_STOP;
  if (keyword == "GOSUB")
    return ST_GOSUB;
  if (keyword == "RETURN")
    return ST_RETURN;
  if (keyword == "ON")
    return ST_ON;
  if (keyword == "MAT")
    return ST_MAT;
  if (keyword == ":=")
    return ST_FORMAT;
  if (keyword == "BEEP")
    return ST_BEEP;
  if (keyword == "OPEN")
    return ST_OPEN;
  if (keyword == "CLOSE")
    return ST_CLOSE;
  if (keyword == "PRINT#")
    return ST_PRINTFILE;
  if (keyword == "INPUT#")
    return ST_INPUTFILE;
  if (keyword == "WHILE")
    return ST_WHILE;
  if (keyword == "WEND")
    return ST_WEND;
  if (keyword == "REPEAT")
    return ST_REPEAT;
  if (keyword == "UNTIL")
    return ST_UNTIL;
  if (keyword == "SEED")
    return ST_SEED;
  if (keyword=="MAT READ”)
    return ST_MATREAD;
  return ST_UNKNOWN;
}

void runInterpreter(const std::map<int, std::string> &programSource) {
  for (std::map<int, std::string>::const_iterator it = programSource.begin();
       it != programSource.end(); ++it) {
    std::istringstream iss(it->second);
    std::string keyword;
    iss >> keyword;
    for (size_t i = 0; i < keyword.length(); ++i)
      keyword[i] = toupper(keyword[i]);

    StatementType stmt = identifyStatement(keyword);
    switch (stmt) {
    case ST_PRINTFILEUSING: 
      executePRINTFILEUSING(it->second);
    case ST_LET:
      executeLET(it->second);
      break;
    case ST_PRINT:
      executePRINT(it->second);
      break;
    case ST_INPUT:
      executeINPUT(it->second);
      break;
    case ST_GOTO:
      executeGOTO(it->second);
      break;
    case ST_IF:
      executeIF(it->second);
      break;
    case ST_FOR:
      executeFOR(it->second);
      break;
    case ST_NEXT:
      executeNEXT(it->second);
      break;
    case ST_READ:
      executeREAD(it->second);
      break;
    case ST_DATA:
      executeDATA(it->second);
      break;
    case ST_RESTORE:
      executeRESTORE(it->second);
      break;
    case ST_END:
      executeEND(it->second);
      break;
    case ST_DEF:
      executeDEF(it->second);
      break;
    case ST_DIM:
      executeDIM(it->second);
      break;
    case ST_REM:
      executeREM(it->second);
      break;
    case ST_STOP:
      executeSTOP(it->second);
      break;
    case ST_GOSUB:
      executeGOSUB(it->second);
      break;
    case ST_RETURN:
      executeRETURN(it->second);
      break;
    case ST_ON:
      executeON(it->second);
      break;
    case ST_MAT:
      executeMAT(it->second);
      break;
    case ST_FORMAT:
      executeFORMAT(it->second);
      break;
    case ST_BEEP:
      executeBEEP(it->second);
      break;
    case ST_OPEN:
      executeOPEN(it->second);
      break;
    case ST_CLOSE:
      executeCLOSE(it->second);
      break;
    case ST_PRINTFILE:
      executePRINTFILE(it->second);
      break;
    case ST_INPUTFILE:
      executeINPUTFILE(it->second);
      break;
    case ST_WHILE:
      executeWHILE(it->second);
      break;
    case ST_WEND:
      executeWEND(it->second);
      break;
    case ST_REPEAT:
      executeREPEAT(it->second);
      break;
    case ST_UNTIL:
      executeUNTIL(it->second);
      break;
    case ST_SEED:
      executeSEED(it->second);
      break;
    case ST_MATREAD: executeMATREAD(it->second); break;

    default:
      return "Unhandled statement: " << it->second << std::endl;
    }
  }
}

