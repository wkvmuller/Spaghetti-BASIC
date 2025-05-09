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
//            Global Variables, structs, etc & helper functions.
//

#define MAX_DIM_ELEMENTES 12
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

long long currentline; // current line we are working on.

std::map<int, std::string> programSource,proghramSourceMaster;

<<<<<<< HEAD
=======
/// Fill a square matrix with 1’s on the diagonal, 0’s elsewhere.


void executeMATIDENTITY(const std::string& line) {
    // Expect syntax:  MAT IDENTITY(A)
    size_t open = line.find('(');
    size_t close = line.find(')', open);
    if (open == std::string::npos || close == std::string::npos) {
        std::cerr << "ERROR: Malformed MAT IDENTITY statement\n";
        return;
    }
    std::string name = line.substr(open + 1, close - open - 1);

    auto it = arrays.find(name);
    if (it == arrays.end()) {
        std::cerr << "ERROR: MAT IDENTITY on undefined matrix " << name << "\n";
        return;
    }
    ArrayInfo& mat = it->second;

    // must be 2D and square
    if (mat.shape.size() != 2 || mat.shape[0] != mat.shape[1]) {
        std::cerr << "ERROR: MAT IDENTITY requires a square 2D matrix: " << name << "\n";
        return;
    }
    int n = mat.shape[0];

    // numeric dense
    if (!mat.data.empty()) {
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                mat.data[i * n + j] = (i == j ? 1.0 : 0.0);
    }
    // numeric sparse
    else {
        mat.sparse.clear();
        for (int i = 0; i < n; ++i)
            mat.sparse[{i, i}] = 1.0;
    }

    // clear any leftover string entries
    mat.stringSparse.clear();
}
>>>>>>> ace41c3 (update pclasss parse...)

// ----- stub helpers ---------------------------------------------------------
void matAdd     (const std::string& dst,const std::string& a,const std::string& b){std::cerr<<"[STUB] matAdd("<<dst<<","<<a<<","<<b<<")\n";}
void matSub     (const std::string& dst,const std::string& a,const std::string& b){std::cerr<<"[STUB] matSub("<<dst<<","<<a<<","<<b<<")\n";}
void matMul     (const std::string& dst,const std::string& a,const std::string& b){std::cerr<<"[STUB] matMul("<<dst<<","<<a<<","<<b<<")\n";}
void matIdentity(const std::string& dst,int n){std::cerr<<"[STUB] matIdentity("<<dst<<","<<n<<")\n";}
void matDeterminant(const std::string& mat,const std::string& dstVar){std::cerr<<"[STUB] matDeterminant("<<mat<<","<<dstVar<<")\n";}

// ----- helpers --------------------------------------------------------------
static inline std::string trim(std::string s){
    auto p = s.find_first_not_of(" \t"); if(p==std::string::npos) return ""; s.erase(0,p);
    p = s.find_last_not_of(" \t"); if(p!=std::string::npos) s.erase(p+1); return s;
}

//
//=======================================================================================
//  MAT functions & assistant routines.
//
//
//==================================================================================
//    MAT Support functions
//

/*
void sparseTrim(ArrayInfo &matrix) {
  for (auto it = matrix.sparse.begin(); it != matrix.sparse.end();) {
    if (std::abs(it->second) < 1e-12)
      it = matrix.sparse.erase(it);
    else
      ++it;
  }
}

double sparseSum(const ArrayInfo &matrix) {
  double total = 0.0;
  for (const auto &[_, val] : matrix.sparse)
    total += val;
  return total;
}

void sparseMultiplyScalar(ArrayInfo &matrix, double scalar) {
  for (auto &[_, val] : matrix.sparse)
    val *= scalar;
}

ArrayInfo sparseMask(const ArrayInfo &source, const ArrayInfo &mask) {
  ArrayInfo result = source;
  result.sparse.clear();
  for (const auto &[key, val] : source.sparse) {
    if (mask.sparse.count(key) && std::abs(mask.sparse.at(key)) > 1e-12)
      result.sparse[key] = val;
  }
  return result;
}
*/

// Helper to parse index string like "A(1,2)" into name and index vector
bool parseIndexedArray(const std::string &token, std::string &name,
                       std::vector<int> &indices) {
  size_t open = token.find('(');
  size_t close = token.find(')');
  if (open == std::string::npos || close == std::string::npos || close < open)
    return false;
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
ArgsInfo getSparseValue(const std::string &name, const std::vector<int> &idx) {
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
void setSparseValue(const std::string &name, const std::vector<int> &idx,
                    const ArgsInfo &value) {
  if (!arrays.count(name)) {
    arrays[name].dimensions = idx.size();
  }
  if (value.isstring) {
    arrays[name].stringSparse[idx] = value.s;
  } else {
    arrays[name].sparse[idx] = value.d;
  }
}

bool invertMatrix(const std::vector<double> &input, std::vector<double> &output,
                  int size) {
  output = input;
  std::vector<double> identity(size * size, 0.0);
  for (int i = 0; i < size; ++i)
    identity[i * size + i] = 1.0;

  for (int col = 0; col < size; ++col) {
    double diag = output[col * size + col];
    if (std::abs(diag) < 1e-12)
      return false;

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

double determinant(const std::vector<double> &mat, int n) {
  if (n == 1)
    return mat[0];
  if (n == 2)
    return mat[0] * mat[3] - mat[1] * mat[2];

  double det = 0.0;
  std::vector<double> submat((n - 1) * (n - 1));
  for (int col = 0; col < n; ++col) {
    int subi = 0;
    for (int i = 1; i < n; ++i) {
      int subj = 0;
      for (int j = 0; j < n; ++j) {
        if (j == col)
          continue;
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

// ─────────────────────────────────────────────────────────────────────────────
// Reads DATA statements into a previously DIM’d matrix (numeric only).
// Usage in BASIC:  MAT READ A
// It will collect all DATA values in program order and load them into A.

void executeMATPRINTFILE(const std::string &line) {
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
    if (!name.empty())
      arraysToPrint.push_back(name);
  }

  // Validate file
  if (!fileHandles.count(filenum) || !fileHandles[filenum].isFileOpen) {
    std::cerr << "ERROR: File #" << filenum << " not open." << std::endl;
    return;
  }
  std::ostream &out = *fileHandles[filenum].stream;

  // For each array name, print all elements row by row
  for (const auto &arrName : arraysToPrint) {
    if (!arrays.count(arrName)) {
      out << "[ERR: " << arrName << " undefined]\n";
      continue;
    }
    const ArrayInfo &mat = arrays[arrName];
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
        if (val.isstring)
          out << val.s;
        else
          out << val.d;
        if (c < cols - 1)
          out << " ";
      }
      out << "\n";
    }
  }
}

void executeMATREAD(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, readWord, name;
  iss >> cmd >> readWord >> name; // “MAT READ A”

  if (!arrays.count(name)) {
    std::cerr << "ERROR: MAT READ undefined matrix " << name << "\n";
    return;
  }
  ArrayInfo &mat = arrays[name];

  // Compute total elements
  size_t total = 1;
  for (int d : mat.shape)
    total *= d;

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

void executeMATPRINT(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, arrayName;
  iss >> cmd >> arrayName;
  std::cout << "[MAT STUB] MAT PRINT " << arrayName << std::endl;
}

void executeMATPRINTFILE(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, hash;
  int filenum;
  iss >> cmd >> hash >> filenum;
  std::string rest;
  std::getline(iss, rest);
  std::cout << "[MAT STUB] MAT PRINT #" << filenum << ", " << rest << std::endl;
}
<<<<<<< HEAD

<<<<<<< HEAD
void executeMAT(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, sub;
  iss >> cmd >> sub; // cmd == "MAT", sub == next token

  if (sub == "READ") {
    // MAT READ X
    executeMATREAD(line);
  } else if (sub == "PRINT") {
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
  } else {
    // MAT <target> = <expr>
    std::string target = sub, eq;
    iss >> eq; // consume “=”
    std::string expr;
    std::getline(iss, expr);
    expr.erase(0, expr.find_first_not_of(" \t"));
    evaluateMATExpression(target, expr);
  }
}
=======
=======
// ─── Hook it into the MAT dispatcher ──────────────────────────────────────────

void void executeMAT(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, sub;
    iss >> cmd >> sub;  // “MAT” and next token
    const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, sub;
    iss >> cmd >> sub;  // cmd == "MAT", sub == next token

    if (sub == "IDENTITY") {
        executeMATIDENTITY(line);
    }
    else if (sub == "READ") {
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
>>>>>>> ace41c3 (update pclasss parse...)
>>>>>>> 79fb27c (restarting interperter)

//=======================================================================================
//   inline functsupport
//

IdentifierReturn evaluateFunction(const std::string &name,
                                  const std::vector<ArgsInfo> &args) {
  IdentifierReturn temp;

  temp.isstring = false; //  all use of temp in this routine is returning a
                         //  double - no string.

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
    if (name == "SIN" || name == "COS" || name == "TAN" || name == "SQR" ||
        name == "STRING$" || name == "LOG" || name == "LOG10" ||
        name == "CLOG" || name == "EXP" || name == "INT" || name == "ROUND" ||
        name == "FLOOR" || name == "CEIL" || name == "RND" || name == "DET") {
      if (args[0].isstring) {
        std::cerr << "Error on " << name
                  << " passing a string where number expected [" << args[0].s
                  << "]  line:" << args[0].linenumber << '\n';
        temp.d = 0.0;
        return temp;
      }
    }

    if (name == "LOGX" || name == "POW") {
      if (args.size() < 2 || args[1].isstring) {
        std::cerr << "Error on " << name << ": bad second argument"
                  << std::endl;
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
      temp.d = static_cast<double>(std::rand()) / static_cast<double> RAND_MAX;
      return temp;
    }

<<<<<<< HEAD
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
          name == "FLOOR" || name == "CEIL" | name == "RND" || name == "DET" )
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
=======
    std::cerr << "Unknown function: " << name << std::endl;
>>>>>>> cb8f062 (Monday update)
    temp.d = 0.0;
    return temp;
  }

  IdentifierReturn evaluateStringFunction(const std::string &name,
                                          const std::vector<ArgsInfo> &args) {
    IdentifierReturn temp;

    temp.isstring =
        true; //  all use  of temp in this routine is returning a  string.

    if (name == "MID$" || name == "TIME$" || name == "DATE$" ||
        name == "CHR$" || name == "LEFT$" || name == "RIGHT$")
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

    if (name == "MID$" || name == "TIME$" || name == "DATE$" ||
        name == "LEFT$" || name == "RIGHT$")
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
    Parser(const std::string &expr, const long long linenumber)
        : input(expr), linenumber(linenumber), pos(0) {}

  private:
    std::string input;
    long long linenumber;
    size_t pos;
    std::string name;
    IdentifierReturn evalueatefunctionreturn;

    double parse() {
      double result = parseExpression();

<<<<<<< HEAD
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
        if (peek() == '"')
          get();

        valreturned.isstring = true;
        valreturned.s = s;
        valreturned.d = 0.0;
        return valreturned;
      }

      // 2) Parenthesized expression: ( expr )
      if (peek() == '(') {
        get(); // consume '('
        double num = parseExpression();
        if (get() != ')')
          throw std::runtime_error("Expected ')' in expression");
        valreturned.isstring = false;
        valreturned.d = num;
        return valreturned;
      }

      // 3) Identifier or function call
      if (std::isalpha(peek())) {
=======
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
>>>>>>> 79fb27c (restarting interperter)
        // parse identifier (may end with $ for string variables)
        size_t start = pos;
        while (pos < input.length() &&
               (std::isalnum(input[pos]) || input[pos] == '$'))
<<<<<<< HEAD
          ++pos;
=======
            ++pos;
>>>>>>> 79fb27c (restarting interperter)
        std::string name = input.substr(start, pos - start);

        // function call?
        if (peek() == '(') {
<<<<<<< HEAD
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
=======
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
>>>>>>> 79fb27c (restarting interperter)
        }

        // not a function: variable lookup
        auto it = variables.find(name);
        if (it != variables.end()) {
<<<<<<< HEAD
          const VarInfo &v = it->second;
          if (v.vT == VT_STRING) {
            valreturned.isstring = true;
            valreturned.s = v.s;
            valreturned.d = 0.0;
          } else {
            valreturned.isstring = false;
            valreturned.d = v.d;
          }
        } else {
          // undefined scalar: default to zero or empty
          valreturned.isstring = (name.back() == '$');
          valreturned.s = "";
          valreturned.d = 0.0;
        }
        return valreturned;
      }

      // 4) Numeric literal
      valreturned.isstring = false;
      valreturned.d = parseNumber();
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
>>>>>>> 79fb27c (restarting interperter)

  //
  //----------------------------------------------------------------------------
  //  functional stubs

  void executeREM(const std::string &) {}

  void executeSTOP(const std::string &) {
    throw std::runtime_error("STOP called");

    void evaluateMATExpression(const std::string &target,
                               const std::string &expression);
    void executeBEEP(const std::string &) {
      std::cout << "Stub of BEEP" << std::endl;
    }
    void executeBEEP(const std::string &) {
      std::cout << "Stub of BEEP" << std::endl;
    }
    void executeCLOSE(const std::string &line) {
      std::cout << "Stub of CLOSE" << std::endl;
    }
    void executeDEF(const std::string &) {
      std::cout << "Stub of DEF" << std::endl;
    }

// ========================= Statement Handlers =========================


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

    if (shape.size() > MAX_DIM_ELEMENTES) {
        std::cerr << "ERROR: Too many dimensions (max "<<MAX_DIM_ELEMENTES<<")." << std::endl;
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
    void executeFOR(const std::string &line) {
      std::cout << "Stub of FOR" << std::endl;
    }
    void executeFORMAT(const std::string &) {
      std::cout << "Stub of FORMAT" << std::endl;
    }
    void executeGO(const std::string &line) {
      std::cout << "Stub of GO" << std::endl;
    }
    void executeGOSUB(const std::string &line) {
      std::cout << "Stub of GOSUB" << std::endl;
    }
    void executeIF(const std::string &) {
      std::cout << "Stub of IF" << std::endl;
    }
    void executeINPUT(const std::string &line) {
      std::cout << "Stub of INPUT" << std::endl;
    }
    void executeINPUTFILE(const std::string &line) {
      std::cout << "Stub of INPUTFILE" << std::endl;
    }
    void executeLET(const std::string &line) {
      std::cout << "Stub of LET" << std::endl;
    }
    void executeMAT(const std::string &line) {
      std::cout << "Stub of MAT" << std::endl;
    }
    void executeMATPRINT(const std::string &line) {
      std::cout << "Stub of MATPRINT" << std::endl;
    }
    void executeMATPRINTFILE(const std::string &line) {
      std::cout << "Stub of MATPRINTFILE" << std::endl;
    }
    void executeMATREAD(const std::string &line) {
      std::cout << "Stub of MATREAD" << std::endl;
    }
    void executeON(const std::string &line) {
      std::cout << "Stub of ON" << std::endl;
    }
    void executeOPEN(const std::string &line) {
      std::cout << "Stub of OPEN" << std::endl;
    }
    void executePRINT(const std::string &line) {
      std::cout << "Stub of PRINT" << std::endl;
    }
    void executePRINTFILE(const std::string &line) {
      std::cout << "Stub of PRINTFILE" << std::endl;
    }
    void executePRINTFILEUSING(const std::string &line) {
      std::cout << "Stub of PRINTFILEUSING" << std::endl;
    }

    void executeREPEAT(const std::string &) {
      std::cout << "Stub of REPEAT" << std::endl;
    }
    void executeRETURN(const std::string &) {
      std::cout << "Stub of RETURN" << std::endl;
    }
    void executeSEED(const std::string &line) {
      std::cout << "Stub of SEED" << std::endl;
    }
<<<<<<< HEAD
    void executeSTOP(const std::string &) {
      std::cout << "Stub of STOP" << std::endl;
=======
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


<<<<<<< HEAD
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

=======
>>>>>>> ace41c3 (update pclasss parse...)


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
>>>>>>> 79fb27c (restarting interperter)
    }

    void executeUNTIL(const std::string &line) {
      std::cout << "Stub of UNTIL" << std::endl;
    }
    void executeWEND(const std::string &) {
      std::cout << "Stub of WEND" << std::endl;
    }
    void executeWHILE(const std::string &line) {
      std::cout << "Stub of WHILE" << std::endl;
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

    default:
      return "Unhandled statement: " << it->second << std::endl;
    }
  }
}

// ---------------------------------------------------------------------------
//  New executeMAT implementation and helper stubs
//  *  Supports three syntactic forms
//      1)  "MAT <M1> = <M2> <op> <M3>"            (binary matrix op)
//      2)  "MAT <M> = IDENTITY(<n>)"               (square identity)
//      3)  "<var> = DET(<M>)"                      (determinant to scalar)
//  *  The heavy‑lifting math is delegated to stub helpers so the runtime
//    builds without pulling in linear‑algebra code right now.
// ---------------------------------------------------------------------------

-------------------------------
void executeMAT(const std::string& line)
{
    std::string src = trim(line);

    // -------------------------------------------
    // Case (3):  "<var> = DET(<matrix>)"  (no "MAT" keyword)
    // -------------------------------------------
    if(src.find("DET(") != std::string::npos && src.find("=") != std::string::npos && src.rfind("MAT",0)!=0)
    {
        size_t eq = src.find('=');
        std::string dstVar   = trim(src.substr(0,eq));
        std::string detExpr  = trim(src.substr(eq+1));       // expects DET(<M>)
        size_t open = detExpr.find('('), close = detExpr.find(')');
        if(open==std::string::npos||close==std::string::npos){
            std::cerr << "ERROR: malformed DET() expression: "<<detExpr<<"\n"; return;}
        std::string matName = trim(detExpr.substr(open+1, close-open-1));
        matDeterminant(matName, dstVar);
        return;
    }

    // At this point we expect the line to start with "MAT "
    if(src.rfind("MAT",0)!=0){
        std::cerr<<"ERROR: executeMAT received non‑MAT line: "<<line<<"\n"; return;}
    std::string rest = trim(src.substr(3));      // after the word MAT

    // -------------------------------------------
    // Case (2):  "<M> = IDENTITY(<n>)"
    // -------------------------------------------
    if(rest.find("IDENTITY(") != std::string::npos){
        size_t eq = rest.find('=');
        std::string dstMat = trim(rest.substr(0,eq));
        size_t open = rest.find('('), close = rest.find(')');
        if(open==std::string::npos||close==std::string::npos){
            std::cerr<<"ERROR: malformed IDENTITY() expression\n"; return;}
        int n = std::stoi(trim(rest.substr(open+1, close-open-1)));
        matIdentity(dstMat,n);
        return;
    }

    // -------------------------------------------
    // Case (1):  binary matrix op  "<M1> = <M2> op <M3>"
    // -------------------------------------------
    size_t eq = rest.find('=');
    if(eq==std::string::npos){ std::cerr<<"ERROR: MAT missing '='\n"; return; }
    std::string dstMat = trim(rest.substr(0,eq));
    std::string rhs    = trim(rest.substr(eq+1));

    std::string lhsMat, rhsMat, op;
    {
        std::istringstream ss(rhs);
        ss >> lhsMat >> op >> rhsMat;
    }
    if(op=="+" )      matAdd(dstMat,lhsMat,rhsMat);
    else if(op=="-" ) matSub(dstMat,lhsMat,rhsMat);
    else if(op=="*" ) matMul(dstMat,lhsMat,rhsMat);
    else {
        std::cerr<<"ERROR: unsupported MAT operator '"<<op<<"'\n";}
}


void evaluateMATExpression(const std::string &target,
                           const std::string &expression) {
  if (expr.find("DET(") == 0) {
    size_t open = expr.find("(");
    size_t close = expr.find(")");
    std::string source = expr.substr(open + 1, close - open - 1);
    if (arrays.find(source) == arrays.end()) {
      std::cerr << "ERROR: Matrix not found: " << source << std::endl;
      return;
    }
    const ArrayInfo &mat = arrays[source];
    if (mat.dimensions != 2 || mat.shape[0] != mat.shape[1]) {
      std::cerr << "ERROR: DETERMINANT requires a square 2D matrix.";
      return;
    }
    double resultVal = determinant(mat.data, mat.shape[0]);
    ArrayInfo result;
    result.dimensions = 2;
    result.shape = {1, 1};
    result.data = {resultVal};
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
      std::cerr << "ERROR: INV source matrix not found: " << source
                << std::endl;
      return;
    }

    const ArrayInfo &src = arrays[source];
    if (src.dimensions != 2 || src.shape.size() != 2 ||
        src.shape[0] != src.shape[1]) {
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
      std::cerr << "ERROR: TRANS source matrix not found: " << source
                << std::endl;
      return;
    }
    const ArrayInfo &src = arrays[source];
    if (src.dimensions != 2 || src.shape.size() != 2) {
      std::cerr << "ERROR: TRANS requires a 2D matrix." << std::endl;
      return;
    }

    ArrayInfo result;
    result.dimensions = 2;
    result.shape = {src.shape[1], src.shape[0]};
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
  if (op == "*" && arrays.find(left) == arrays.end() &&
      arrays.find(right) != arrays.end()) {
    // SCALAR * MATRIX
    double scalar = std::stod(left);
    const ArrayInfo &mat = arrays[right];
    ArrayInfo result = mat;
    if (mat.dimensions >= 4) {
      for (auto &[key, val] : result.sparse) {
        val *= scalar;
      }
    } else {
      for (auto &val : result.data) {
        val *= scalar;
      }
    }
    arrays[target] = result;
    return;
  } else if (op == "*" && arrays.find(left) != arrays.end() &&
             arrays.find(right) == arrays.end()) {
    // MATRIX * SCALAR
    double scalar = std::stod(right);
    const ArrayInfo &mat = arrays[left];
    ArrayInfo result = mat;
    if (mat.dimensions >= 4) {
      for (auto &[key, val] : result.sparse) {
        val *= scalar;
      }
    } else {
      for (auto &val : result.data) {
        val *= scalar;
      }
    }
    arrays[target] = result;
    return;
  }

  iss >> token1;

  if (iss >> op >> token2) {
    if (arrays.find(token1) == arrays.end() ||
        arrays.find(token2) == arrays.end()) {
      std::cerr << "ERROR: One or both matrices not defined: " << token1 << ", "
                << token2 << std::endl;
      return;
    }

    const ArrayInfo &a = arrays[token1];
    const ArrayInfo &b = arrays[token2];

    if (a.dimensions != b.dimensions || a.shape != b.shape) {
      std::cerr << "ERROR: Dimension mismatch in MAT operation." << std::endl;
      return;
    }

    ArrayInfo result;
    result.dimensions = a.dimensions;
    result.shape = a.shape;

    if (a.dimensions >= 4) {
      for (const auto &entry : a.sparse) {
        if (b.sparse.find(entry.first) != b.sparse.end()) {
          if (op == "+") {
            result.sparse[entry.first] =
                entry.second + b.sparse.at(entry.first);
          } else if (op == "-") {
            result.sparse[entry.first] =
                entry.second - b.sparse.at(entry.first);
          } else if (op == "*") {
            result.sparse[entry.first] =
                entry.second * b.sparse.at(entry.first);
          } else {
            std::cerr << "ERROR: Unsupported operator " << op
                      << " in sparse matrix." << std::endl;
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
          std::cerr << "ERROR: Unsupported operator " << op
                    << " in dense matrix." << std::endl;
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
