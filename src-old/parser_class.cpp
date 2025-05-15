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

<<<<<<< HEAD
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
            args.push_back(
                makeArgsInfo(linenumber, name, false, "", parseExpression()));
          } while (peek() == ',' && get());
        }
        if (get() != ')') {
          std::string errstr =
              "Expected ')' after function args: " + std::to_string(linenumber);
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
  }
  else {
    evalueatefunctionreturn.isstring = false;
    evalueatefunctionreturn.d = parseNumber();
    return evalueatefunctionreturn;
  }
=======
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
>>>>>>> ace41c3 (update pclasss parse...)

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
