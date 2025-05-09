#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>


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

<<<<<<< HEAD

//
//=======================================================================================
//  MAT functions & assistant routines.
//
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


// ─────────────────────────────────────────────────────────────────────────────
// Reads DATA statements into a previously DIM’d matrix (numeric only).
// Usage in BASIC:  MAT READ A
// It will collect all DATA values in program order and load them into A.


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


=======
// name → (parameterName, expressionString)
std::map<std::string, std::pair<std::string, std::string>> userFns;

/
//=======================================================================================
//   inline functsupport
//
/
>>>>>>> 79fb27c (restarting interperter)
//=======================================================================================
//   inline functsupport
//

IdentifierReturn evaluateFunction(const std::string &name,
                                  const std::vector<ArgsInfo> &args) {
  IdentifierReturn temp;

<<<<<<< HEAD
  temp.isstring = false; //  all use of temp in this routine is returning a
                         //  double - no string.
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>

// Forward declaration of ArgsInfo and IdentifierReturn
struct ArgsInfo {
    long long linenumber;
    std::string identifiername;
    bool isstring;
    std::string s;
    double d;
};
struct IdentifierReturn {
    bool isstring;
    std::string s;
    double d;
};

IdentifierReturn evaluateFunction(const std::string &name,
                                  const std::vector<ArgsInfo> &args) {
    IdentifierReturn temp;
    temp.isstring = false; // this function returns a double

    if (name == "ASCII") {
        if (!args[0].isstring || args[0].s.empty()) {
            std::cerr << "Bad string passed to ASCII(" << args[0].s
                      << ") line:" << args[0].linenumber << std::endl;
            temp.d = 0.0;
            return temp;
        } else {
            temp.d = static_cast<double>(static_cast<unsigned char>(args[0].s[0]));
            return temp;
        }
    }

    if (name == "LEN$") {
        if (!args[0].isstring) {
            std::cerr << "Bad non-string passed to LEN$(" << args[0].d
                      << ") line:" << args[0].linenumber << std::endl;
            temp.d = -1;
            return temp;
        } else {
            temp.d = static_cast<double>(args[0].s.length());
            return temp;
        }
    }

    if ((name == "SIN" || name == "COS" || name == "TAN" || name == "SQR" ||
         name == "LOG" || name == "LOG10" || name == "EXP" || name == "INT" ||
         name == "ROUND" || name == "FLOOR" || name == "CEIL") && args[0].isstring) {
        std::cerr << "Error on " << name
                  << ": string passed where number expected [" << args[0].s
                  << "] line:" << args[0].linenumber << std::endl;
        temp.d = 0.0;
        return temp;
    }

    if (name == "LOGX" || name == "POW") {
        if (args.size() < 2 || args[1].isstring) {
            std::cerr << "Error on " << name 
                      << ": bad second argument" << std::endl;
            temp.d = 0.0;
            return temp;
        }
    }

    if (name == "LOGX") {
        temp.d = std::log(args[1].d) / std::log(args[0].d);
        return temp;
    }
    if (name == "SIN") {
        temp.d = std::sin(args[0].d);
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
    if (name == "LOG10") {
        temp.d = std::log10(args[0].d);
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
        temp.d = std::pow(args[0].d, args[1].d);
        return temp;
    }
    if (name == "RND") {
        temp.d = static_cast<double>(std::rand()) / RAND_MAX;
        return temp;
    }

    std::cerr << "Unknown function: " << name << std::endl;
    temp.d = 0.0;
    return temp;
}
=======
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
  // }
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
  if (name == "RND") {temp.d = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);}

  if (name == "ASIN") {
    temp.d = std::asin(args[0].d);
    return temp;
  }
  if (name == "ACOS") {
    temp.d = std::acos(args[0].d);
    return temp;
  }
  if (name == "ATAN") {
    temp.d = std::atan(args[0].d);
    return temp;
  }
  if (name == "COT") {
    temp.d = 1.0 / std::tan(args[0].d);
    return temp;
  }
  if (name == "SEC") {
    temp.d = 1.0 / std::cos(args[0].d);
    return temp;
  }
  if (name == "CSC") {
    temp.d = 1.0 / std::sin(args[0].d);
    return temp;
  }
  if (name == "DEG2RAD") {
    temp.d = args[0].d * M_PI / 180.0;
    return temp;
  }
  if (name == "RAD2DEG") {
    temp.d = args[0].d * 180.0 / M_PI;
    return temp;
  }
  if (name == "DET") {
    std::cerr << "DET() not implemented - placeholder only."<<__FILE__<<":"__LINE__ << std::endl;
    temp.d = 0.0;
    return temp;
  }
    temp.d = rand() / RAND_MAX;
    return temp;
  }
  std::cerr << "Unknown function: " << name << std::endl;
  temp.d = static_cast<double>(0.0);
  return temp;
}

>>>>>>> 79fb27c (restarting interperter)
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


<<<<<<< HEAD
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

IdentifierReturn Parser::parsePrimary() {
    IdentifierReturn valreturned;
    skipWhitespace();

    // 1) String literal: "..."
    if (peek() == '"') {
        // consume opening quote
        get();
        std::string s;
        // accumulate until closing quote or end
        while (pos < input.length() && input[pos] != '"') {
            s += input[pos++];
        }
        // consume closing quote if present
        if (peek() == '"') get();

        valreturned.isstring = true;
        valreturned.s        = s;
        valreturned.d        = 0.0;
        return valreturned;
    }

    // 2) Parenthesized expression: ( expr )
    if (peek() == '(') {
        get();  // consume '('
        double num = parseExpression();
        if (get() != ')')
            throw std::runtime_error("Expected ')' in expression");
        valreturned.isstring = false;
        valreturned.d        = num;
        return valreturned;
    }

    // 3) Identifier or function call
    if (std::isalpha(peek())) {
        // parse identifier (may end with $ for string variables)
        size_t start = pos;
        while (pos < input.length() &&
               (std::isalnum(input[pos]) || input[pos] == '$'))
            ++pos;
        std::string name = input.substr(start, pos - start);

        // function call?
        if (peek() == '(') {
            // consume '('
            get();
            std::vector<ArgsInfo> args;
            if (peek() != ')') {
                // parse comma‐separated expressions
                do {
                    double argval = parseExpression();
                    args.push_back(makeArgsInfo(linenumber, name, false, "", argval));
                } while (peek() == ',' && get());
            }
            if (get() != ')')
                throw std::runtime_error("Expected ')' after function arguments");

            // dispatch to numeric or string function evaluator
            if (name.back() == '$') {
                // string‐returning function
                IdentifierReturn tmp = evaluateStringFunction(name, args);
                return tmp;
            } else {
                IdentifierReturn tmp = evaluateFunction(name, args);
                return tmp;
            }
        }

        // not a function: variable lookup
        auto it = variables.find(name);
        if (it != variables.end()) {
            const VarInfo &v = it->second;
            if (v.vT == VT_STRING) {
                valreturned.isstring = true;
                valreturned.s        = v.s;
                valreturned.d        = 0.0;
            } else {
                valreturned.isstring = false;
                valreturned.d        = v.d;
            }
        } else {
            // undefined scalar: default to zero or empty
            valreturned.isstring = (name.back() == '$');
            valreturned.s        = "";
            valreturned.d        = 0.0;
        }
        return valreturned;
    }

    // 4) Numeric literal
    valreturned.isstring = false;
    valreturned.d        = parseNumber();
    return valreturned;
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

=======
>>>>>>> 79fb27c (restarting interperter)
//
//----------------------------------------------------------------------------
//  functional stubs

void executeREM(const std::string &) {}

void executeSTOP(const std::string &) { throw std::runtime_error("STOP called");

void evaluateMATExpression(const std::string& target, const std::string& expression);
void executeBEEP(const std::string &) { std::cout << "Stub of BEEP" << std::endl; }
void executeBEEP(const std::string &) { std::cout << "Stub of BEEP" << std::endl; }
void executeCLOSE(const std::string& line) { std::cout << "Stub of CLOSE" << std::endl; }
void executeDEF(const std::string &) { std::cout << "Stub of DEF" << std::endl; }
void executeDEF(const std::string &);}
void executeDIM(const std::string &line) { std::cout << "Stub of DIM" << std::endl; }
void executeFOR(const std::string &line) { std::cout << "Stub of FOR" << std::endl; }
void executeFORMAT(const std::string &) { std::cout << "Stub of FORMAT" << std::endl; }
void executeGO(const std::string &line) { std::cout << "Stub of GO" << std::endl; }
void executeGOSUB(const std::string &line) { std::cout << "Stub of GOSUB" << std::endl; }
void executeIF(const std::string &) { std::cout << "Stub of IF" << std::endl; }
void executeINPUT(const std::string &line) { std::cout << "Stub of INPUT" << std::endl; }
void executeINPUTFILE(const std::string& line) { std::cout << "Stub of INPUTFILE" << std::endl; }
void executeLET(const std::string& line) { std::cout << "Stub of LET" << std::endl; }
void executeMAT(const std::string& line) { std::cout << "Stub of MAT" << std::endl; }
void executeMATPRINT(const std::string& line) { std::cout << "Stub of MATPRINT" << std::endl; }
void executeMATPRINTFILE(const std::string& line) { std::cout << "Stub of MATPRINTFILE" << std::endl; }
void executeMATREAD(const std::string& line) { std::cout << "Stub of MATREAD" << std::endl; }
void executeON(const std::string &line) { std::cout << "Stub of ON" << std::endl; }
void executeOPEN(const std::string& line) { std::cout << "Stub of OPEN" << std::endl; }
void executePRINT(const std::string& line) { std::cout << "Stub of PRINT" << std::endl; }
void executePRINTFILE(const std::string& line) { std::cout << "Stub of PRINTFILE" << std::endl; }
void executePRINTFILEUSING(const std::string& line) { std::cout << "Stub of PRINTFILEUSING" << std::endl; }
void executeREM(const std::string &) { std::cout << "Stub of REM" << std::endl; }
void executeREM(const std::string &);}
void executeREPEAT(const std::string&) { std::cout << "Stub of REPEAT" << std::endl; }
void executeRETURN(const std::string &) { std::cout << "Stub of RETURN" << std::endl; }
void executeSEED(const std::string& line) { std::cout << "Stub of SEED" << std::endl; }
void executeSTOP(const std::string &) { std::cout << "Stub of STOP" << std::endl; }
void executeSTOP(const std::string &) { std::cout << "Stub of STOP" << std::endl; }
void executeUNTIL(const std::string& line) { std::cout << "Stub of UNTIL" << std::endl; }
void executeWEND(const std::string&) { std::cout << "Stub of WEND" << std::endl; }
void executeWHILE(const std::string& line) { std::cout << "Stub of WHILE" << std::endl; }

<<<<<<< HEAD

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

=======
>>>>>>> 79fb27c (restarting interperter)
