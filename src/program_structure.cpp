#include "program_structure.h"

// Define the global instance
PROGRAM_STRUCTURE program;

int currentLine = 0; // Current BASIC execution line

//
//======================================================================
// General support functions.
//

std::string trim(const std::string &s) {
  const char *WS = " \t\r\n";
  size_t b = s.find_first_not_of(WS), e = s.find_last_not_of(WS);
  return (b == std::string::npos) ? "" : s.substr(b, e - b + 1);
}

std::string evalStringExpression(const std::string &expr) {
  size_t pos = 0;
  auto skipWS = [&]() {
    while (pos < expr.size() && std::isspace((unsigned char)expr[pos]))
      ++pos;
  };
  skipWS();
  // 1) String literal
  if (pos < expr.size() && expr[pos] == '"') {
    ++pos;
    size_t start = pos;
    while (pos < expr.size() && expr[pos] != '"')
      ++pos;
    if (pos >= expr.size())
      throw std::runtime_error("SYNTAX ERROR: Unterminated string literal");
    std::string lit = expr.substr(start, pos - start);
    ++pos;
    skipWS();
    if (pos != expr.size())
      throw std::runtime_error(
          "SYNTAX ERROR: Trailing characters after string literal");
    return lit;
  }

  // 2) Identifier or function call
  if (pos < expr.size() && std::isalpha((unsigned char)expr[pos])) {
    // read name (can include letters, digits, underscore, trailing $)
    size_t start = pos;
    while (pos < expr.size() && (std::isalnum((unsigned char)expr[pos]) ||
                                 expr[pos] == '_' || expr[pos] == '$'))
      ++pos;
    std::string id = expr.substr(start, pos - start);
    skipWS();
    // Function call?
    if (pos < expr.size() && expr[pos] == '(') {
      ++pos;
      skipWS();
      // parse arguments separated by commas until ')'
      std::vector<std::string> args;
      size_t argStart = pos;
      int depth = 0;
      while (pos < expr.size()) {
        if (expr[pos] == '(') {
          depth++;
        } else if (expr[pos] == ')') {
          if (depth == 0)
            break;
          depth--;
        } else if (expr[pos] == ',' && depth == 0) {
          args.push_back(trim(expr.substr(argStart, pos - argStart)));
          ++pos;
          skipWS();
          argStart = pos;
          continue;
        }
        ++pos;
      }
      args.push_back(trim(expr.substr(argStart, pos - argStart)));
      if (pos >= expr.size() || expr[pos] != ')')
        throw std::runtime_error(
            "SYNTAX ERROR: Missing ')' in string function call");
      ++pos;
      skipWS();
      if (pos != expr.size())
        throw std::runtime_error(
            "SYNTAX ERROR: Trailing characters after function call");

      // Dispatch known functions
      if (id == "LEFT$") {
        if (args.size() != 2)
          throw std::runtime_error("SYNTAX ERROR: LEFT$ requires 2 args");
        std::string s = evalStringExpression(args[0]);
        int n = static_cast<int>(std::stoi(args[1]));
        return s.substr(0, n);
      }
      if (id == "RIGHT$") {
        if (args.size() != 2)
          throw std::runtime_error("SYNTAX ERROR: RIGHT$ requires 2 args");
        std::string s = evalStringExpression(args[0]);
        int n = static_cast<int>(std::stoi(args[1]));
        if (n >= (int)s.size())
          return s;
        return s.substr(s.size() - n);
      }
      if (id == "MID$") {
        if (args.size() != 3)
          throw std::runtime_error("SYNTAX ERROR: MID$ requires 3 args");
        std::string s = evalStringExpression(args[0]);
        int i = static_cast<int>(std::stoi(args[1])) - 1;
        int n = static_cast<int>(std::stoi(args[2]));
        if (i < 0)
          i = 0;
        if (i >= (int)s.size())
          return "";
        return s.substr(i, n);
      }
      if (id == "LEN$") {
        if (args.size() != 1)
          throw std::runtime_error("SYNTAX ERROR: LEN$ requires 1 arg");
        std::string s = evalStringExpression(args[0]);
        return std::to_string(s.length());
      }
      if (id == "CHR$") {
        if (args.size() != 1)
          throw std::runtime_error("SYNTAX ERROR: CHR$ requires 1 arg");
        int code = std::stoi(args[0]);
        return std::string(1, static_cast<char>(code));
      }
      if (id == "STRING$") {
        if (args.size() < 1 || args.size() > 2)
          throw std::runtime_error(
              "SYNTAX ERROR: STRING$ requires 1 or 2 args");
        int count = std::stoi(args[0]);
        char fill = ' ';
        if (args.size() == 2) {
          std::string f = evalStringExpression(args[1]);
          if (f.empty())
            fill = ' ';
          else
            fill = f[0];
        }
        return std::string(count, fill);
      }
      if (id == "TIME$") {
        if (!args.empty())
          throw std::runtime_error("SYNTAX ERROR: TIME$ takes no args");
        std::time_t t = std::time(nullptr);
        std::tm tm;
        localtime_r(&t, &tm);
        char buf[9];
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm.tm_hour, tm.tm_min,
                      tm.tm_sec);
        return buf;
      }
      if (id == "DATE$") {
        if (!args.empty())
          throw std::runtime_error("SYNTAX ERROR: DATE$ takes no args");
        std::time_t t = std::time(nullptr);
        std::tm tm;
        localtime_r(&t, &tm);
        char buf[11];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", tm.tm_year + 1900,
                      tm.tm_mon + 1, tm.tm_mday);
        return buf;
      }
      throw std::runtime_error("RUNTIME ERROR: Unknown string function: " + id);
    }

    // 3) Variable lookup
    if (!id.empty() && id.back() == '$') {
      std::string name = id.substr(0, id.size() - 1);
      auto it = program.stringVariables.find(name);
      if (it != program.stringVariables.end())
        return it->second.stringValue;
      return ""; // undefined strings default to empty
    }
  }

  throw std::runtime_error("SYNTAX ERROR: Invalid string expression: " + expr);
}

double evalExpression(const std::string &expr) {
  size_t pos = 0;
  auto skipWS = [&]() {
    while (pos < expr.size() && std::isspace((unsigned char)expr[pos]))
      ++pos;
  };

  std::function<double()> parseExpr, parseTerm, parseFactor, parsePrimary;

  // expr = term { (+|-) term }
  parseExpr = [&]() -> double {
    double value = parseTerm();
    skipWS();
    while (pos < expr.size()) {
      char op = expr[pos];
      if (op == '+' || op == '-') {
        ++pos;
        skipWS();
        double rhs = parseTerm();
        value = (op == '+' ? value + rhs : value - rhs);
        skipWS();
      } else
        break;
    }
    return value;
  };

  // term = factor { (*|/) factor }
  parseTerm = [&]() -> double {
    double value = parseFactor();
    skipWS();
    while (pos < expr.size()) {
      char op = expr[pos];
      if (op == '*' || op == '/') {
        ++pos;
        skipWS();
        double rhs = parseFactor();
        if (op == '/' && rhs == 0.0)
          throw std::runtime_error("Division by zero");
        value = (op == '*' ? value * rhs : value / rhs);
        skipWS();
      } else
        break;
    }
    return value;
  };

  // factor = [ - ] ( number | '(' expr ')' | function | variable )
  parseFactor = [&]() -> double {
    skipWS();
    bool neg = false;
    if (pos < expr.size() && expr[pos] == '-') {
      neg = true;
      ++pos;
      skipWS();
    }
    double value = 0.0;
    if (pos < expr.size() && expr[pos] == '(') {
      ++pos;
      skipWS();
      value = parseExpr();
      skipWS();
      if (pos >= expr.size() || expr[pos] != ')')
        throw std::runtime_error("Missing closing parenthesis");
      ++pos;
    } else if (pos < expr.size() &&
               (std::isdigit((unsigned char)expr[pos]) || expr[pos] == '.')) {
      size_t start = pos;
      while (pos < expr.size() &&
             (std::isdigit((unsigned char)expr[pos]) || expr[pos] == '.'))
        ++pos;
      if (pos < expr.size() && (expr[pos] == 'e' || expr[pos] == 'E')) {
        ++pos;
        if (pos < expr.size() && (expr[pos] == '+' || expr[pos] == '-'))
          ++pos;
        while (pos < expr.size() && std::isdigit((unsigned char)expr[pos]))
          ++pos;
      }
      try {
        value = std::stod(expr.substr(start, pos - start));
      } catch (...) {
        throw std::runtime_error("Invalid number: " +
                                 expr.substr(start, pos - start));
      }
    } else {
      value = parsePrimary();
    }
    return neg ? -value : value;
  };

  // primary = identifier [ '(' expr ')' ] | error
  parsePrimary = [&]() -> double {
    skipWS();
    if (pos >= expr.size() || !std::isalpha((unsigned char)expr[pos]))
      throw std::runtime_error(std::string("Unexpected character '") +
                               (pos < expr.size() ? expr[pos] : '?') +
                               "' in expression");
    // read identifier
    size_t start = pos;
    while (pos < expr.size() &&
           (std::isalnum((unsigned char)expr[pos]) || expr[pos] == '_'))
      ++pos;
    std::string id = expr.substr(start, pos - start);
    std::string idUp = id;
    for (auto &c : idUp)
      c = std::toupper((unsigned char)c);

    skipWS();
    // function call?
    if (pos < expr.size() && expr[pos] == '(') {
      ++pos;
      skipWS();
      double arg = parseExpr();
      skipWS();
      if (pos >= expr.size() || expr[pos] != ')')
        throw std::runtime_error("Missing ')' after argument to " + id);
      ++pos;
      // check user-defined FN<name>
      if (idUp.rfind("FN", 0) == 0) {
        std::string name = idUp.substr(2);
        auto it = program.userFunctions.find(name);
        if (it != program.userFunctions.end()) {
          std::string body = it->second.expr;
          std::string param = it->second.param;
          std::string argStr = std::to_string(arg);

          size_t p = 0;
          while ((p = body.find(param, p)) != std::string::npos) {
            // replace exactly param.length() characters with argStr
            body.replace(p, param.length(), argStr);
            // advance past what we just inserted
            p += argStr.size();
          }
          return evalExpression(body);
        }
      }
      // built-in single-arg functions
      if (idUp == "SIN")
        return std::sin(arg);
      if (idUp == "COS")
        return std::cos(arg);
      if (idUp == "TAN")
        return std::tan(arg);
      if (idUp == "ATN")
        return std::atan(arg);
      if (idUp == "EXP")
        return std::exp(arg);
      if (idUp == "LOG10")
        return std::log10(arg);
      if (idUp == "CLOG")
        return std::log(arg);
      if (idUp == "SQR")
        return std::sqrt(arg);
      if (idUp == "INT")
        return std::floor(arg);
      if (idUp == "RND")
        return std::rand() / (double)RAND_MAX;
      if (idUp == "DEG2RAD")
        return arg * 3.14159265358979323846 / 180.0;
      if (idUp == "RAD2DEG")
        return arg * 180.0 / 3.14159265358979323846;
      throw std::runtime_error("Unknown function: " + id);
    }

    // variable lookup
    auto itNum = program.numericVariables.find(id);
    if (itNum != program.numericVariables.end()) {
      return itNum->second.numericValue;
    }
    auto itStr = program.stringVariables.find(id);
    if (itStr != program.stringVariables.end()) {
      try {
        return std::stod(itStr->second.stringValue);
      } catch (...) {
        throw std::runtime_error("Cannot convert string to number: " + id);
      }
    }

    throw std::runtime_error("Unknown identifier: " + id);
  };

  skipWS();
  double result = parseExpr();
  skipWS();
  if (pos != expr.size())
    throw std::runtime_error("Unexpected trailing text: " + expr.substr(pos));
  return result;
}


// Evaluates expr and returns its integer value.
// Supports +, -, *, / and parentheses. Throws on syntax error.
int evalIntExpr(const std::string &expr) {
  size_t pos = 0;

  // Skip whitespace
  auto skipWS = [&]() {
    while (pos < expr.size() && isspace(expr[pos]))
      ++pos;
  };

  // Forward declarations
  std::function<long()> parseExpr, parseTerm, parseFactor;

  // <expression> ::= <term> { (+|-) <term> }
  parseExpr = [&]() -> long {
    long value = parseTerm();
    skipWS();
    while (pos < expr.size()) {
      if (expr[pos] == '+') {
        ++pos;
        skipWS();
        value += parseTerm();
      } else if (expr[pos] == '-') {
        ++pos;
        skipWS();
        value -= parseTerm();
      } else
        break;
      skipWS();
    }
    return value;
  };

  // <term> ::= <factor> { (*|/) <factor> }
  parseTerm = [&]() -> long {
    long value = parseFactor();
    skipWS();
    while (pos < expr.size()) {
      if (expr[pos] == '*') {
        ++pos;
        skipWS();
        value *= parseFactor();
      } else if (expr[pos] == '/') {
        ++pos;
        skipWS();
        long rhs = parseFactor();
        if (rhs == 0)
          throw std::runtime_error("Division by zero");
        value /= rhs;
      } else
        break;
      skipWS();
    }
    return value;
  };

  // <factor> ::= [-] ( number | '(' <expression> ')' )
  parseFactor = [&]() -> long {
    skipWS();
    bool neg = false;
    if (pos < expr.size() && expr[pos] == '-') {
      neg = true;
      ++pos;
      skipWS();
    }
    long value = 0;
    if (pos < expr.size() && expr[pos] == '(') {
      ++pos; // consume '('
      value = parseExpr();
      skipWS();
      if (pos >= expr.size() || expr[pos] != ')')
        throw std::runtime_error("Missing closing parenthesis");
      ++pos;
    } else if (pos < expr.size() && isdigit(expr[pos])) {
      while (pos < expr.size() && isdigit(expr[pos])) {
        value = value * 10 + (expr[pos++] - '0');
      }
    } else {
      throw std::runtime_error("Invalid factor in expression");
    }
    return neg ? -value : value;
  };

  // Parse and ensure we've consumed everything
  long result = parseExpr();
  skipWS();
  if (pos != expr.size())
    throw std::runtime_error("Unexpected characters in expression");
  return static_cast<int>(result);
}

