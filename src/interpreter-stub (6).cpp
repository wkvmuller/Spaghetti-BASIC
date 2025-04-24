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

struct IdentifierReturn {
  bool isstring;
  std::string s;
  double d;
};

long long currentline;  //current line we are working on.

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

void executeLET(const std::string& line) {
    std::istringstream iss(line);
    std::string keyword, target, eq;
    iss >> keyword >> target >> eq;
    std::string expr;
    std::getline(iss, expr);
    expr.erase(0, expr.find_first_not_of(" 	"));

    ArgsInfo value = evaluateFunction("VALUE", {makeArgsInfo(expr)});

    std::string name;
    std::vector<int> indices;
    if (parseIndexedArray(target, name, indices)) {
        setSparseValue(name, indices, value);
    } else {
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

    std::cout << target << " = " << (value.isstring ? value.s : std::to_string(value.d)) << std::endl;
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
   else {
    double value = evaluateExpression(expr, currentline);info.vT = VT_DOUBLE;
    info.vT = VT_DOUBLE;
    info.d = value;
    variables[target] = info;
    std::cout << target << " = " << value << std::endl;
  }
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
      std::cout <<  prompt << " ";
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
    std::cout << var << "? ";
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

void executeGO(const std::string &line) { std::cerr << "[STUB] executeGO called\n"; }
 else {
    std::cerr << "ERROR: GO to undefined line " << target << std::endl;
    currentLineNumber = -1;
  }
}

void executeIF(const std::string &) { return "[IF stub]\n"; }
void executeFOR(const std::string &line) { std::cerr << "[STUB] executeFOR called\n"; }


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

void executeDEF(const std::string &) { std::cerr << "[STUB] executeDEF called\n"; }


void executeDIM(const std::string &line) { std::cerr << "[STUB] executeDIM called\n"; }
 catch (...) {
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

void executeREM(const std::string &) { std::cerr << "[STUB] executeREM called\n"; }


void executeSTOP(const std::string &) { std::cerr << "[STUB] executeSTOP called\n"; }


void executeGOSUB(const std::string &line) { }// std::cerr << "[STUB] executeGOSUB called\n"; }
/*
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
  } */
}

void executeRETURN(const std::string &) { std::cerr << "[STUB] executeRETURN called\n"; }
 /* else {
    currentLineNumber = gosubStack.top();
    gosubStack.pop();
  } */
}

void executeON(const std::string &line) { std::cerr << "[STUB] executeON called\n"; }


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
    std::string cmd, target, equals;
    iss >> cmd >> target >> equals;
    std::string expression;
    std::getline(iss, expression);
    expression.erase(0, expression.find_first_not_of(" 	"));
    evaluateMATExpression(target, expression);
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

void executeFORMAT(const std::string &) { std::cerr << "[STUB] executeFORMAT called\n"; }


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

void executeBEEP(const std::string &) { std::cerr << "[STUB] executeBEEP called\n"; }


void executeOPEN(const std::string &) { std::cerr << "[STUB] executeOPEN called\n"; }

void executeCLOSE(const std::string &) { std::cerr << "[STUB] executeCLOSE called\n"; }

void executePRINTFILE(const std::string &) { std::cerr << "[STUB] executePRINTFILE called\n"; }

void executeINPUTFILE(const std::string &) { std::cerr << "[STUB] executeINPUTFILE called\n"; }


void executeWHILE(const std::string& line) { std::cerr << "[STUB] executeWHILE called\n"; }
/*

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

    loopStack.push_back({"WHILE", cond, currentLineNumber}); */
}

void executeWEND(const std::string&) { std::cerr << "[STUB] executeWEND called\n"; }
/*

    LoopFrame frame = loopStack.back();
    if (evaluateExpression(frame.condition) != 0.0) {
        currentLineNumber = frame.returnLine;
    } else {
        loopStack.pop_back();
    } */
}

void executeREPEAT(const std::string&) { std::cerr << "[STUB] executeREPEAT called\n"; }


//    loopStack.push_back({"REPEAT", "", currentLineNumber});
}

void executeUNTIL(const std::string& line) { std::cerr << "[STUB] executeUNTIL called\n"; }
/*

    std::string cond = line.substr(5);
    cond.erase(0, cond.find_first_not_of(" \t"));

    if (evaluateExpression(cond) == 0.0) {
        currentLineNumber = loopStack.back().returnLine;
    } else {
        loopStack.pop_back();
    } */
}

void executeSEED(const std::string& line) { std::cerr << "[STUB] executeSEED called\n"; }


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
  ST_SEED
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
