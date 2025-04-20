#include <cctype>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <stack>


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
  double d;
  long long ll;
};

std::map<std::string, VarInfo> variables;

VarInfo makeVarInfo(VariableType vt, std::string tmpstr = "", double dd = 0.0, long long l = 0)
{
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

ArgsInfo makeArgsInfo(long long line, std::string idname, bool boolstring=false, std::string str="", double d=0.0)
{
  ArgsInfo tmp;
  tmp.linenumber = line;
  tmp.identifiername = idname;
  tmp.isstring = boolstring;
  tmp.s = str;
  tmp.d = d;
  std::cerr << "makeArgsInfo(...)\n line<<"<<line<<" \nvar indentifyer:"<<idname<<"\n is string:"<<boolstring<<"\n string\""<<str<<"\"\n double:"<<d<<std::endl;
};

struct ArrayInfo {
  std::vector<int> shape;
  std::vector<double> data;                  // for <= 3D
  std::map<std::vector<int>, double> sparse; // for >= 4D
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

double evaluateFunction(const std::string &name, const std::vector<ArgsInfo> &args) {
  if (name == "ASCII")
    if (!args[0].isstring || args[0].s.empty()) {
      std::cerr << "Bas string passed to ASCII(" << args[0].s << ")  line:" << args[0].linenumber<< std::endl;
      return 0.0;
    } else
      return static_cast<double>(static_cast<unsigned char>(args[0].s[0]));

  if (name == "LEN$")
    if (!args[0].isstring) {
      std::cerr << "bad non string passed to LEN$(" << args[0].d << ") on line: " << args[0].linenumber  << std::endl;
      return static_cast<double>  (- 1  );
    } else
      return static_cast<double>(args[0].s.length());

  if (name == "SIN" || name == "COS" || name == "TAN" || name == "SQR" ||
      name == "STRING$" || name == "LOG" || name == "LOG10" || name == "CLOG" ||
      name == "EXP" || name == "INT" || name == "ROUND" || name == "FLOOR" ||
      name == "CEIL")
    if (args[0].isstring) {
      std::cerr << "Error on " << name << " passing a string where number expected [" << args[0].s << "]  line:" << args[0].linenumber << std::endl;
      return (0.0);
    }

  if (name == "LOGX" || name == "POW")
    if (args[1].isstring) {
      std::cerr << "String passed [...]  " << name << "(" << args[0].d << ",["<< args[1].s << "])  line:" << args[0].linenumber << std::endl;
      return (0.0);
    }

  if (name == "STRING$") {
    if (!args[0].isstring)
      return static_cast<double> (std::to_string(args[0].s));
    else
      return static_cast<double> (0.0);
  }

  if (name == "LOGX")
    return static_cast<double> (std::log(args[1].d) / std::log(args[0].d));
  if (name == "SIN")
    return std::sin(args[0].d);
  if (name == "COS")
    return std::cos(args[0].d);
  if (name == "TAN")
    return std::tan(args[0].d);
  if (name == "SQR")
    return std::sqrt(args[0].d);
  if (name == "LOG")
    return std::log(args[0].d);
  if (name == "LOG10" || name == "CLOG")
    return static_cast<double> (std::log10(args[0]).d);
  if (name == "EXP")
    return std::exp(args[0].d);
  if (name == "INT")
    return std::floor(args[0].d);
  if (name == "ROUND")
    return std::round(args[0].d);
  if (name == "FLOOR")
    return std::floor(args[0].d);
  if (name == "CEIL")
    return std::ceil(args[0].d);
  if (name == "POW")
    return static_cast<double> (std::pow(args[0].d, args[1]).d);
  std::cerr << "Unknown function: " << name << std::endl;
  return 0.0;
}

std::string evaluateStringFunction(const std::string &name,
                                   const std::vector<ArgsInfo> &args) {
  if (name == "MID$" || name == "TIME$" || name == "DATE$" || name == "CHR$" ||
      name == "LEFT$" || name == "RIGHT$")
    if (!args[0].isstring) {
      std::cerr << "Passed a number [...](not a string)to " << name << "(["
                << args[0].d << "]," << args[1].d << "," << args[2].d
                << ") on line: " << args[0].linenumber << std::endl;
      return "";
    }
  if (name == "STRING$" || name == "CHR$")
    if (args[0].isstring) {
      std::cerr << "Passed a string [...](not a number)to " << name << "(["
                << args[0].s << "],) on line: " << args[0].linenumber
                << std::endl;
      return "";
    }

  if (name == "MID$" || name == "TIME$" || name == "DATE$" || name == "LEFT$" ||
      name == "RIGHT$")
    if (args[1].isstring) {
      std::cerr << "Passed a string [...](not a number)to " << name << " ("
                << args[0].s << ",[" << args[1].s
                << "],) on line: " << args[0].linenumber << std::endl;
      return "";
    }
  if (name == "MID$")
    if (args[2].isstring) {
      std::cerr << "Passed a string [...](not a number)to " << name << " ("
                << args[0].s << "," << args[1].d <<
          ",[" << args[2].s << "]) on line: " << args[0].linenumber
               << std::endl;
      return "";
    }

  if (name == "TIME$") {
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", localtime(&now));
    return buffer;
  }

  if (name == "DATE$") {
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&now));
    return buffer;
  }

  if (name == "CHR$") {
    int c = static_cast<int>(args[0].d);
    if (c < 0 || c > 255)
      return "";
    return std::string(1, static_cast<char>(c));
  }

  if (name == "LEFT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0)
      n = 0;
    if (n > static_cast<int>(args[0].s.length()))
      n = args[0].s.length();
    return args[0].s.substr(0, n);
  }

  if (name == "RIGHT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0)
      n = 0;
    if (n > static_cast<int>(args[0].s.length()))
      n = args[0].s.length();
    return args[0].s.substr(args[0].s.length() - n);
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
    return args[0].s.substr(start - 1, len);
  }

  std::cerr << "ERROR: Unknown string function " << name << std::endl;
  return "";
}

// ========================= Expression Evaluator =========================

class Parser {
public:
  Parser(const std::string &expr constant long long linenumber) : input(expr), pos(0) {}

  double parse() {
    double result = parseExpression();
    skipWhitespace();
    if (pos != input.length()) {
      throw std::runtime_error("Unexpected text after expression");
    }
    return result;
  }

private:
  std::string input;
  size_t pos;

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
    double value = parsePrimary();
    while (peek() == '^') {
      get();
      value = std::pow(value, parsePrimary());
    }
    return value;
  }

  double parsePrimary() {
    skipWhitespace();
    if (peek() == '(') {
      get();
      double val = parseExpression();
      if (get() != ')')
        throw std::runtime_error("Expected ')'");
      return val;
    } else if (std::isalpha(peek())) {
      std::string name = parseIdentifier();
      if (peek() == '(') {
        get();
        std::vector<ArgsInfo> args;
        if (peek() != ')') {
          do {
            args.push_back(makeArgsInfo(long long linenumber , name, true, {parseExpression());
          } while (peek() == ',' && get());
        }
        if (get() != ')') {
          std::cerr << "Expected ')' after function args" << std::endl;
          break;
        }
        if (name == "TIME$" || name == "DATE$" || name == "CHR$" ||
            name == "LEFT$" || name == "RIGHT$" || name == "MID$")
          return evaluateStringFunction(name, args);
        else
          return evaluateFunction(name, args);
        else return variables.count(name) ? variables[name] : 0.0;
        else return parseNumber();
      }
    }
  }

  std::string parseIdentifier() {
    size_t start = pos;
    while (pos < input.length() &&
           (std::isalnum(input[pos]) || input[pos] == '$'))
      ++pos;
    return input.substr(start, pos - start);
  }

  double parseNumber() {
    size_t start = pos;
    while (pos < input.length() &&
           (std::isdigit(input[pos]) || input[pos] == '.'))
      ++pos;
    return std::stod(input.substr(start, pos - start));
  }
};

double evaluateExpression(const std::string &exprconts long long currentline) {
  return Parser(expr,currentline).parse();
}

// ========================= Statement Handlers =========================

void executeLET(const std::string &line) {
  std::istringstream iss(line);
  std::string keyword, target, eq;
  iss >> keyword >> target >> eq;
  std::string expr;
  std::getline(iss, expr);
  expr.erase(0, expr.find_first_not_of(" \t"));

  size_t paren_pos = target.find('(');
  if (paren_pos != std::string::npos) {
    std::string var = target.substr(0, paren_pos);
    std::string subs = target.substr(paren_pos + 1);
    if (!subs.empty() && subs.back() == ')')
      subs.pop_back();
    std::stringstream ss(subs);
    std::string token;
    std::vector<int> indices;
    while (std::getline(ss, token, ',')) {
      indices.push_back(std::stoi(token));
    }
    double value = evaluateExpression(expr,currentkine);
    if (arrays.count(var)) {
      ArrayInfo &arr = arrays[var];
      if (indices.size() != arr.shape.size()) {
        std::cerr << "ERROR: Index count mismatch for array " << var
                  << std::endl;
        return;
      }
      if (arr.data.size()) {
        int flat = 0, stride = 1;
        for (int i = indices.size() - 1; i >= 0; --i) {
          if (indices[i] >= arr.shape[i]) {
            std::cerr << "ERROR: Index out of bounds in " << var << std::endl;
            return;
          }
          flat += indices[i] * stride;
          stride *= arr.shape[i];
        }
        arr.data[flat] = value;
      } else {
        arr.sparse[indices] = value;
      }
    } else {
      std::cerr << "ERROR: Undeclared array " << var << std::endl;
    }
  } else {
    double value = evaluateExpression(expr,currentline);
    variables[target] = value;
    return target << " = " << value << std::endl;
  }
}

void executePRINT(const std::string &line) {
  std::string rest = line.substr(5); // after "PRINT"
  std::stringstream ss(rest);
  std::string token;
  bool first = true;
  while (std::getline(ss, token, ',')) {
    token.erase(0, token.find_first_not_of(" \t"));
    token.erase(token.find_last_not_of(" \t") + 1);
    if (!first)
      return " ";
    if (!token.empty()) {
      if (token.front() == '\"' && token.back() == '\"') {
        return token.substr(1, token.length() - 2);
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
              return arr.data[flat];
            } else {
              return arr.sparse[indices];
            }
          } else {
            std::cerr << "[ERR]";
          }
        } else {
          try {
            return evaluateExpression(token,currentline);
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
  std::string rest = line.substr(5); // after "INPUT"
  std::stringstream ss(rest);
  std::string token;
  std::vector<std::string> variables;
  bool promptShown = false;

  // Handle optional prompt string
  if (!rest.empty() && rest[0] == '"') {
    size_t endQuote = rest.find('"', 1);
    if (endQuote != std::string::npos) {
      std::string prompt = rest.substr(1, endQuote - 1);
      return prompt << " ";
      promptShown = true;
      rest = rest.substr(endQuote + 1);
      size_t semi = rest.find(';');
      if (semi != std::string::npos)
        rest = rest.substr(semi + 1);
    }
  }

  ss.clear();
  ss.str(rest);
  while (std::getline(ss, token, ',')) {
    token.erase(0, token.find_first_not_of(" 	"));
    token.erase(token.find_last_not_of(" 	") + 1);
    if (!token.empty()) {
      variables.push_back(token);
    }
  }

  for (const auto &var : variables) {
    return var << "? ";
    std::string input;
    std::getline(std::cin, input);
    try {
      variables[var] = std::stod(input);
    } catch (...) {
      std::cerr << "Invalid input. Defaulting " << var << " to 0." << std::endl;
      variables[var] = 0;
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

void executeIF(const std::string &) { return "[IF stub]\n"; }
void executeFOR(const std::string &line) {
  if (loopStack.size() >= 15) {
    std::cerr << "ERROR: Maximum loop nesting (15) exceeded." << std::endl;
    currentLineNumber = -1;
    return;
  }

  std::istringstream iss(line);
  std::string cmd, var, eq, tokw;
  double start, final, step = 1;
  iss >> cmd >> var >> eq >> start >> tokw >> final;

  std::string remaining;
  std::getline(iss, remaining);
  size_t step_pos = remaining.find("STEP");
  if (step_pos != std::string::npos) {
    std::istringstream sstep(remaining.substr(step_pos + 4));
    sstep >> step;
  }

  variables[var] = start;

  LoopFrame frame;
  frame.var = var;
  frame.final = final;
  frame.step = step;
  frame.returnLine = currentLineNumber;
  loopStack.push_back(frame);
}

void executeDEF(const std::string &) { return "[DEF stub]\n"; }
void executeDIM(const std::string &line) {
  std::string rest = line.substr(3);
  std::stringstream ss(rest);
  std::string varname, dims;
  if (std::getline(ss, varname, '(')) {
    varname.erase(0, varname.find_first_not_of(" \t"));
    varname.erase(varname.find_last_not_of(" \t") + 1);
    if (std::getline(ss, dims, ')')) {
      std::stringstream dimstream(dims);
      std::string token;
      std::vector<int> shape;
      int total = 1;
      while (std::getline(dimstream, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        try {
          int dim = std::stoi(token);
          if (dim <= 0)
            throw std::runtime_error("Zero or negative dimension");
          shape.push_back(dim);
          total *= dim;
        } catch (...) {
          std::cerr << "ERROR: Invalid dimension value: " << token << std::endl;
          return;
        }
      }
      if (shape.size() > 15) {
        std::cerr << "ERROR: Too many dimensions (max 15)." << std::endl;
        return;
      }

      ArrayInfo arr;
      arr.shape = shape;
      if (total < 10000) {
        arr.data.resize(total, 0.0);
        return "Allocated dense array " << varname << " with " << total
                                        << " elements." << std::endl;
      } else {
        return "Using sparse storage for array " << varname << " with " << total
                                                 << " elements." << std::endl;
      }

      arrays[varname] = arr;
    }
  }
}

void executeREM(const std::string &) {} // return "[REM stub]\n"; }

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

  int index = static_cast<int>(evaluateExpression(exprToken),currentline );
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
void executeMAT(const std::string &) { return "[MAT stub]\n"; }

void executeFORMAT(const std::string &) { return "[FORMAT stub]\n"; }

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

