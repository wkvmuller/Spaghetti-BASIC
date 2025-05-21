#include "interpreter.h"
#include "program_structure.h"

extern PROGRAM_STRUCTURE program;

extern int currentLine;

std::map<std::string, VarInfo> variables;

//
//--------------------------------------------------------------------------------
//             prototypes
//
//
// void evaluateMATExpression(const std::string &target,
//                           const std::string &expression);
// void executeBEEP(const std::string &);
// void executeCLOSE(const std::string &line);
// void executeDEF(const std::string &);
// void executeDIM(const std::string &line);
// void executeFOR(const std::string &line);
// void executeFORMAT(const std::string &);
// void executeGO(const std::string &line);
void executeGOTO(const std::string &line);
void executeGOSUB(const std::string &line);
// void executeIF(const std::string &);
void executeLET(const std::string &line);
void executeMATops(const std::string &line);
// void executeMATPRINT(const std::string &line);
// void executeMATPRINTFILE(const std::string &line);
// void executeMATREAD(const std::string &line);
// void executeON(const std::string &line);
// void executeOPEN(const std::string &line);
// void executeREM(const std::string &);
// void executeREPEAT(const std::string &);
// void executeRETURN(const std::string &);
void executeSEED(const std::string &line);
void executeSTOP(const std::string &);
// void executeUNTIL(const std::string &line);
// void executeWEND(const std::string &);
// void executeWHILE(const std::string &line);

extern void executeFORMAT(const std::string &line);
extern void executePRINTFILE(const std::string &line);
extern void executeINPUTops(const std::string &line);
extern void executeOPEN(const std::string &line);
extern void executeCLOSE(const std::string &line);
extern double evaluateFunction(const std::string &name,
                               const std::vector<ArgsInfo> &args);
extern std::string evaluateStringFunction(const std::string &name,
                                          const std::vector<ArgsInfo> &args);
extern void executeINPUT(const std::string &line);
extern void executeINPUTFILE(const std::string &line);
extern void executePRINTexpr(const std::string &line);
extern void executePRINTFILEUSING(const std::string &line);
// extern ArgsInfo makeArgsInfo(long long line, std::string idname, bool boolstring = false, std::string str = "", double d = 0.0);
extern void executeMATPRINT(const std::string &line,
                            std::ostream &out = std::cout);
extern void executeMATPRINTFILE(const std::string &line);
extern void executeMAT(const std::string &line);
//
//=========================================================================
//  Statments support.
//

// extern PROGRAM_STRUCTURE program;
// OPEN statement: OPEN "filename" FOR INPUT|OUTPUT|APPEND AS #<channel>

// —————————————————————————————————————————————————————————
// DATA statement: parses DATA <datum>{,<datum>}
// Stores into program.dataValues
// —————————————————————————————————————————————————————————
void executeDATA(const std::string &line) {
  static const std::regex rgx(R"(^\s*DATA\s+(.*)\s*$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid DATA: " + line);

  std::string list = m[1].str();
  std::stringstream ss(list);
  std::string token;
  while (std::getline(ss, token, ',')) {
    // trim
    token = std::regex_replace(token, std::regex(R"(^\s+|\s+$)"), "");
    VarInfo v;
    if (token.size() >= 2 && token.front() == '\"' && token.back() == '\"') {
      v.isString = true;
      v.stringValue = token.substr(1, token.size() - 2);
    } else {
      v.isString = false;
      v.numericValue = std::stod(token);
    }
    v.isArray = false;
    program.dataValues.push_back(v);
  }
}

// —————————————————————————————————————————————————————————
// READ statement: READ var1,var2$,...
// Pulls from program.dataValues in order
// —————————————————————————————————————————————————————————
void executeREAD(const std::string &line) {
  static const std::regex rgx(
      R"(^\s*READ\s+([A-Z][A-Z0-9_]{0,31}\$?(?:\s*,\s*[A-Z][A-Z0-9_]{0,31}\$?)*)\s*$)",
      std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid READ: " + line);

  std::stringstream ss(m[1].str());
  std::string tok;
  while (std::getline(ss, tok, ',')) {
    tok = std::regex_replace(tok, std::regex(R"(^\s+|\s+$)"), "");
    bool wantsString = (tok.back() == '$');
    std::string name = wantsString ? tok.substr(0, tok.size() - 1) : tok;

    if (program.dataPointer >= program.dataValues.size())
      throw std::runtime_error("RUNTIME ERROR: No more DATA");

    VarInfo val = program.dataValues[program.dataPointer++];
    if (wantsString) {
      // Allocate or fetch the VarInfo slot for this string var
      VarInfo &var = program.stringVariables[name];
      // Store the DATA value as a string
      var.stringValue =
          val.isString ? val.stringValue : std::to_string(val.numericValue);
      var.isString = true;
    } else {
      // Allocate or fetch the VarInfo slot for this numeric var
      VarInfo &var = program.numericVariables[name];
      // Store the DATA value as a number
      var.numericValue =
          !val.isString ? val.numericValue : std::stod(val.stringValue);
      var.isString = false;
    }
  }
}

// —————————————————————————————————————————————————————————
// RESTORE statement: resets DATA pointer
// —————————————————————————————————————————————————————————
void executeRESTORE(const std::string & /*line*/) { program.dataPointer = 0; }

// BEEP statement — emit a bell character
void executeBEEP(const std::string & /*line*/) {
  std::cout << '\a' << std::flush;
}

// DEF FN<name>(<param>) = <expression>
void executeDEF(const std::string &line) {
  static const std::regex rgx(
      R"(^\s*DEF\s+FN([A-Z][A-Z0-9_]{0,31})\s*\(\s*([A-Z][A-Z0-9_]{0,31})\s*\)\s*=\s*(.+)$)",
      std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid DEF: " + line);
  }

  std::string name = m[1].str();  // function name
  std::string param = m[2].str(); // single parameter
  std::string expr = m[3].str();  // body expression

  // Store or overwrite
  program.userFunctions[name] = UserFunction{param, expr};
}

void executeEND(const std::string &line) {
  throw std::runtime_error("RUNTIME ERROR: END of program");
}

// Assumes you have a helper to eval an arithmetic expression to an int:

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

void executeDIM(const std::string &line) {
  // Strip off the "DIM" keyword
  std::string rest = line.substr(3);
  std::smatch m;
  // Match:  identifier  =  [A–Z][A–Z0–9_]{0,31} with optional trailing '$'
  //           dims       =  anything inside the parentheses
  static const std::regex rgx(
      R"(^\s*([A-Z][A-Z0-9_]{0,31}\$?)\s*\(([^)]*)\)\s*$)", std::regex::icase);

  if (!std::regex_match(rest, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid DIM syntax: " + line);
  }

  // Extract name and detect string vs numeric
  std::string name = m[1].str();
  bool isString = false;
  if (name.back() == '$') {
    isString = true;
    name.pop_back();
  }

  // Split and evaluate each dimension expression
  std::vector<int> dims;
  std::stringstream ss(m[2].str());
  std::string part;
  while (std::getline(ss, part, ',')) {
    int size = evalIntExpr(part);
    if (size < 0) {
      throw std::runtime_error("RUNTIME ERROR: Negative array size in DIM");
    }
    dims.push_back(size);
  }

  // Enforce maximum of 15 dimensions
  if (dims.size() > 15) {
    throw std::runtime_error("SYNTAX ERROR: DIM exceeds 15 dimensions: " +
                             std::to_string(dims.size()));
  }

  // Compute total elements
  size_t totalElements = 1;
  for (int d : dims)
    totalElements *= d;

  // Perform the allocation in either stringMatrices or numericMatrices
  if (isString) {
    auto &mat = program.stringMatrices[name];
    mat.dimensions = dims;
    mat.configureStorage(totalElements);
  } else {
    auto &mat = program.numericMatrices[name];
    mat.dimensions = dims;
    mat.configureStorage(totalElements);
  }
}

// FOR handler: FOR <var> = <start> TO <end> [STEP <step>]
void executeFOR(const std::string &line) {
  static const std::regex rgx(
      R"(\\s*FOR\\s+([A-Z][A-Z0-9_]{0,31})\\s*=\\s*(.+?)\\s+TO\\s+(.+?)(?:\\s+STEP\\s+(.+))?\\s*$)",
      std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid FOR: " + line);
  }
  std::string var = m[1].str();
  double start = evalExpression(m[2].str());
  double end = evalExpression(m[3].str());
  double step = m[4].matched ? evalExpression(m[4].str()) : 1.0;

  if (program.forStack.size() >= 15) {
    throw std::runtime_error("RUNTIME ERROR: FOR nesting exceeds 15 levels");
  }

  // Initialize loop variable
  VarInfo &v = program.numericVariables[var];
  v.numericValue = start;
  v.isString = false;

  // Push loop info
  ForInfo fi{var, end, step, program.currentLine};
  program.forStack.push_back(fi);
}

/**
 * dispatchStatement
 *
 * Given a single BASIC statement (without its line number),
 * invoke the appropriate executeXXX handler.
 */
void dispatchStatement(const std::string &stmt) {
  // Extract the first word (keyword)
  std::istringstream iss(stmt);
  std::string kw;
  iss >> kw;
  // Uppercase it
  std::transform(kw.begin(), kw.end(), kw.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  if (kw == "LET") {
    executeLET(stmt);
  }
  //   else if (kw == "DEF") {
  //        executeDEF(stmt);
  //    }
  //    else if (kw == "DIM") {
  //        executeDIM(stmt);
  //   }
  //    else if (kw == "DATA") {
  //        executeDATA(stmt);
  //    }
  else if (kw == "READ") {
    executeREAD(stmt);
  } else if (kw == "RESTORE") {
    executeRESTORE(stmt);
  } else if (kw == "PRINT") {
    executePRINTexpr(stmt);
  } else if (kw == "INPUT") {
    executeINPUTops(stmt);
  } else if (kw == "GOTO") {
    executeGOTO(stmt);
  } else if (kw == "GOSUB") {
    executeGOSUB(stmt);
  }
  //    else if (kw == "RETURN") {
  //        executeRETURN(stmt);
  //    }
  //    else if (kw == "ON") {
  //        executeON(stmt);
  //    }
  //    else if (kw == "IF") {
  //        executeIF(stmt);
  //    }
  //    else if (kw == "FOR") {
  //        executeFOR(stmt);
  //    }
  //    else if (kw == "NEXT") {
  //        executeNEXT(stmt);
  //    }
  //    else if (kw == "WHILE") {
  //        executeWHILE(stmt);
  //    }
  //    else if (kw == "WEND") {
  //        executeWEND(stmt);
  //    }
  //    else if (kw == "REPEAT") {
  //       executeREPEAT(stmt);
  //    }
  //    else if (kw == "UNTIL") {
  //        executeUNTIL(stmt);
  //    }
  else if (kw == "MAT") {
    executeMATops(stmt);
  } else if (kw == "SEED") {
    executeSEED(stmt);
  } else if (kw == "STOP") {
    executeSTOP(stmt);
  } else if (kw == "END") {
    executeEND(stmt);
  }
  //    else if (kw == "FORMAT") {
  //        executeFORMAT(stmt);
  //    }
  else {
    throw std::runtime_error("SYNTAX ERROR: Unknown statement: " + kw + ":" +
                             stmt);
  }
}

/**
 * IF handler: single‐line IF…THEN
 *
 * Syntax: IF <expression> THEN <statement>
 * Evaluates the expression; if non-zero, executes the trailing statement.
 */
void executeIF(const std::string &line) {
  static const std::regex rgx(R"(^\s*IF\s+(.+?)\s+THEN\s+(.+)$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid IF syntax: " + line);
  }

  std::string expr = m[1].str();
  std::string stmt = m[2].str();

  // Evaluate the condition
  double cond = evalExpression(expr);
  if (cond != 0.0) {
    // Dispatch the embedded statement (e.g. GOTO 100, PRINT "Hi", etc.)
    dispatchStatement(stmt);
  }
}

// LET statement: LET <var> = <expr>
void executeLET(const std::string &line) {
  static const std::regex rgx(
      R"(^\s*LET\s+([A-Z][A-Z0-9_]{0,31}\$?)\s*=\s*(.+)$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid LET syntax: " + line);
  }

  std::string varName = m[1].str();
  bool isString = false;
  if (!varName.empty() && varName.back() == '$') {
    isString = true;
    varName.pop_back();
  }

  std::string expr = m[2].str();
  if (isString) {
    // Evaluate as string expression
    std::string val = evalStringExpression(expr);
    VarInfo &slot = program.stringVariables[varName];
    slot.stringValue = val;
    slot.isString = true;
  } else {
    // Evaluate as numeric expression
    double val = evalExpression(expr);
    VarInfo &slot = program.numericVariables[varName];
    slot.numericValue = val;
    slot.isString = false;
  }

static const std::regex letRe(R"(^LET\s+([A-Z][A-Z0-9_]*)\((\d+),\s*(\d+)\)\s*=\s*(.+)$)", std::regex::icase);

if (std::regex_match(line, m, letRe)) {
    std::string name = m[1];
    int row = std::stoi(m[2]);
    int col = std::stoi(m[3]);
    double val = evalExpression(m[4]);

    VarInfo v;
    v.numericValue = val;
    v.isString = false;

    program.matrices[name].set({row, col}, v);
}
}

/**
 * MAT READ handler.
 *
 * Reads values from the DATA pool into a numeric matrix.
 * Syntax: MAT READ <matrixName>
 */
void executeMATREAD(const std::string &line) {
  static const std::regex rgx(R"(^\s*MAT\s+READ\s+([A-Z][A-Z0-9_]{0,31})\s*$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid MAT READ: " + line);
  }

  std::string name = m[1].str();
  auto it = program.numericMatrices.find(name);
  if (it == program.numericMatrices.end()) {
    throw std::runtime_error("RUNTIME ERROR: Matrix not defined: " + name);
  }
  MatrixValue &mv = it->second;

  // Compute total elements = product of dimensions
  size_t total = 1;
  for (size_t i = 0; i < mv.dimensions.size(); ++i) {
    total *= static_cast<size_t>(mv.dimensions[i]);
  }
  // Configure dense vs sparse storage
  mv.configureStorage(total);

  // Fill matrix from DATA values
  for (size_t idx = 0; idx < total; ++idx) {
    if (program.dataPointer >= program.dataValues.size()) {
      throw std::runtime_error(
          "RUNTIME ERROR: Out of DATA while reading matrix " + name);
    }
    const VarInfo &dv = program.dataValues[program.dataPointer++];
    if (!mv.isSparse) {
      mv.denseValues[idx] = dv;
    } else {
      // Compute multi-dimensional index for sparse storage
      MatrixIndex mi;
      mi.dimensions.resize(mv.dimensions.size());
      size_t tmp = idx;
      for (int d = static_cast<int>(mv.dimensions.size()) - 1; d >= 0; --d) {
        size_t dimSize = static_cast<size_t>(mv.dimensions[d]);
        mi.dimensions[d] = static_cast<int>(tmp % dimSize);
        tmp /= dimSize;
      }
      mv.sparseValues[mi] = dv;
    }
  }
}
/**
 * Dispatch all MAT‐related statements:
 *
 *   MAT <id> = <matexpr>             → executeMAT
 *   MAT READ <id>                     → executeMATREAD
 *   MAT PRINT #<chan>, <id1>,<id2>    → executeMATPRINTFILE
 *   MAT PRINT <id1>,<id2>,…           → executeMATPRINT
 */
void executeMATops(const std::string &line) {
  static const std::regex assignRe(R"(^\s*MAT\s+([A-Z][A-Z0-9_]*)\s*=\s*(.+)$)",
                                   std::regex::icase);
  static const std::regex readRe(R"(^\s*MAT\s+READ\s+([A-Z][A-Z0-9_]*)\s*$)",
                                 std::regex::icase);
  static const std::regex printFileRe(
      R"(^\s*MAT\s+PRINT\s*#\s*(\d+)\s*,\s*(.+)$)", std::regex::icase);
  static const std::regex printRe(R"(^\s*MAT\s+PRINT\s+(.+)$)",
                                  std::regex::icase);

  std::smatch m;
  if (std::regex_match(line, m, assignRe)) {
    // MAT <id> = <matexpr>
    executeMAT(line);
  } else if (std::regex_match(line, m, readRe)) {
    // MAT READ <id>
    executeMATREAD(line);
  } else if (std::regex_match(line, m, printFileRe)) {
    // MAT PRINT #<chan>, <id list>
    executeMATPRINTFILE(line);
  } else if (std::regex_match(line, m, printRe)) {
    // MAT PRINT <id list>
    executeMATPRINT(line, std::cout);
  } else {
    throw std::runtime_error("SYNTAX ERROR: Invalid MAT statement: " + line);
  }
}
// FORMAT statement: defines a format string for PRINT USING
// Syntax:  <line> := "format-spec"
// e.g.    100 := "###,###.###   lllllllllll   cccccc    rrrrrrr"
// Corrected executeFORMAT regex with proper raw string delimiter
static const std::regex rgx(R"FMT(^\s*(\d+)\s*:=\s*"([^"]*)"\s*$)FMT",
                            std::regex::icase);

// PRINT USING handler
// Syntax: PRINT USING <formatLine> <var1>,<var2$>,...
// Optional output stream overload
// Helper to find a line in programSource or throw
static std::map<int, std::string>::const_iterator findLine(int ln) {
  auto it = program.programSource.find(ln);
  if (it == program.programSource.end())
    throw std::runtime_error("RUNTIME ERROR: Undefined line " +
                             std::to_string(ln));
  return it;
}

// —————————————————————————————————————————————
// GOTO <n>
// —————————————————————————————————————————————
void executeGOTO(const std::string &line) {
  static const std::regex rgx(R"(^\s*GOTO\s+(\d+)\s*$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid GOTO: " + line);
  int target = std::atoi(m[1].str().c_str());
  // Verify target exists
  findLine(target);
  // Schedule the jump
  program.nextLineNumber = target;
  program.nextLineNumberSet = true;
}

// —————————————————————————————————————————————
// GOSUB <n>
// —————————————————————————————————————————————
void executeGOSUB(const std::string &line) {
  static const std::regex rgx(R"(^\s*GOSUB\s+(\d+)\s*$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid GOSUB: " + line);
  if (program.gosubStack.size() >= 15)
    throw std::runtime_error("RUNTIME ERROR: GOSUB nesting exceeds 15 levels");

  int target = std::atoi(m[1].str().c_str());
  // Verify target exists
  findLine(target);

  // Push return address (the *next* line) onto stack
  //  extern int currentLine;
  program.gosubStack.push_back(currentLine);

  // Schedule jump
  program.nextLineNumber = target;
  program.nextLineNumberSet = true;
}

// —————————————————————————————————————————————
// RETURN
// —————————————————————————————————————————————
void executeRETURN(const std::string & /*line*/) {
  if (program.gosubStack.empty())
    throw std::runtime_error("RUNTIME ERROR: RETURN without GOSUB");
  int retLine = program.gosubStack.back();
  program.gosubStack.pop_back();
  // Verify return line still exists
  findLine(retLine);

  program.nextLineNumber = retLine;
  program.nextLineNumberSet = true;
}

// —————————————————————————————————————————————
// ON <expr> GOTO|GOSUB <list>
// —————————————————————————————————————————————
void executeON(const std::string &line) {
  static const std::regex rgx(
      R"(^\s*ON\s+(.+?)\s+(GOTO|GOSUB)\s+(\d+(?:\s*,\s*\d+)*)\s*$)",
      std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid ON: " + line);

  // Evaluate the selector expression (1-based index)
  double d = evalExpression(m[1].str());
  int idx = static_cast<int>(d);
  if (idx < 1)
    throw std::runtime_error("RUNTIME ERROR: ON index must be >=1");

  // Build target list
  std::vector<int> targets;
  std::stringstream ss(m[3].str());
  std::string tok;
  while (std::getline(ss, tok, ',')) {
    int t = std::atoi(tok.c_str());
    // Verify each target exists
    findLine(t);
    targets.push_back(t);
  }
  if (idx > (int)targets.size())
    throw std::runtime_error("RUNTIME ERROR: ON index out of range");

  const std::string verb = m[2].str();
  int chosen = targets[idx - 1];

  if (verb == "GOTO" || verb == "goto" || verb == "Goto") {
    program.nextLineNumber = chosen;
    program.nextLineNumberSet = true;
  } else {
    // GOSUB branch
    if (program.gosubStack.size() >= 15)
      throw std::runtime_error(
          "RUNTIME ERROR: GOSUB nesting exceeds 15 levels");

    program.gosubStack.push_back(currentLine);
    program.nextLineNumber = chosen;
    program.nextLineNumberSet = true;
  }
}

void executeREM(const std::string &line) { std::string mivic = line; }

// SEED <unsigned-integer>
void executeSEED(const std::string &line) {
  static const std::regex rgx(R"(^\s*SEED\s+(\d+)\s*$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid SEED: " + line);
  }
  unsigned int seed = static_cast<unsigned int>(std::stoul(m[1].str()));
  std::srand(seed);
  program.seedValue = seed;
}

void executeSTOP(const std::string &line) {
  throw std::runtime_error("RUNTIME ERROR: STOP encountered");
}

/**
 * REPEAT handler.
 *   REPEAT
 * Marks the start of a repeat/until loop.
 */
void executeREPEAT(const std::string &line) {
  static const std::regex rgx(R"(^\\s*REPEAT\\s*$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid REPEAT: " + line);
  }
  // Push current line number onto stack
  program.repeatStack.push_back(program.currentLine);
}

/**
 * UNTIL handler.
 *   UNTIL <expression>
 * Evaluates the expression; if false (zero), loops back to REPEAT.
 * Otherwise exits the loop.
 */
void executeUNTIL(const std::string &line) {
  static const std::regex rgx(R"(^\\s*UNTIL\\s+(.+)$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid UNTIL: " + line);
  }
  if (program.repeatStack.empty()) {
    throw std::runtime_error("RUNTIME ERROR: UNTIL without REPEAT");
  }
  // Get condition expression
  std::string cond = m[1].str();
  double value = evalExpression(cond);
  if (value == 0.0) {
    // false: go back to REPEAT
    int startLine = program.repeatStack.back();
    program.nextLineNumber = startLine;
    program.nextLineNumberSet = true;
  } else {
    // true: exit loop
    program.repeatStack.pop_back();
  }
}

/**
 * WEND handler
 *   WEND
 *
 * Pops the top WHILE from loopStack, re-evaluates its condition,
 * and either jumps back to the WHILE line or exits the loop.
 */
void executeWEND(const std::string &line) {
  static const std::regex rgx(R"(^\s*WEND\s*$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid WEND: " + line);
  }
  if (program.loopStack.empty()) {
    throw std::runtime_error("RUNTIME ERROR: WEND without WHILE");
  }
  // C++11‐style unpack of the top WHILE
  auto loopInfo = program.loopStack.back();
  std::string cond = loopInfo.first;
  int startLine = loopInfo.second;

  double value = evalExpression(cond);
  if (value != 0.0) {
    // repeat
    program.nextLineNumber = startLine;
    program.nextLineNumberSet = true;
  } else {
    // exit
    program.loopStack.pop_back();
  }
}

void executeWHILE(const std::string &line) {
  static const std::regex rgx(R"(^\s*WHILE\s+(.+)$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid WHILE: " + line);
  }
  std::string cond = m[1].str();
  double value = evalExpression(cond);
  if (value != 0.0) {
    // true: enter loop
    program.loopStack.push_back({cond, program.currentLine});
  } else {
    // false: skip loop body
    // Interpreter must skip lines until matching WEND
    // (skipping logic handled elsewhere)
  }
}

// NEXT handler: NEXT <var>
void executeNEXT(const std::string &line) {
  static const std::regex rgx(R"(\\s*NEXT\\s+([A-Z][A-Z0-9_]{0,31})\\s*$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid NEXT: " + line);
  }
  std::string var = m[1].str();

  if (program.forStack.empty()) {
    throw std::runtime_error("RUNTIME ERROR: NEXT without FOR");
  }

  // Check that top of stack matches
  ForInfo fi = program.forStack.back();
  if (fi.varName != var) {
    throw std::runtime_error(
        "RUNTIME ERROR: NEXT variable mismatch: expected " + fi.varName);
  }

  // Update loop variable
  VarInfo &v = program.numericVariables[var];
  double val = v.numericValue + fi.step;
  v.numericValue = val;

  // Check loop termination
  bool done = (fi.step > 0.0 ? val > fi.endValue : val < fi.endValue);
  if (done) {
    // Pop loop and continue
    program.forStack.pop_back();
  } else {
    // Repeat: jump back to just after the FOR line
    program.nextLineNumber = fi.forLine;
    program.nextLineNumberSet = true;
  }
}

// ========================= Dispatcher =========================

enum StatementType {
  ST_UNKNOWN,
  ST_LET,
  ST_PRINTexpr,
  ST_INPUTops,
  ST_GOTO,
  ST_IF,
  ST_FOR,
  ST_NEXT,
  ST_READ,
  ST_DATA,
  ST_RESTORE,
  ST_DEF,
  ST_DIM,
  ST_REM,
  ST_STOP,
  ST_GOSUB,
  ST_RETURN,
  ST_END,
  ST_ON,
  ST_PRINTFILEUSING,
  ST_MAT,
  ST_FORMAT,
  ST_BEEP,
  ST_OPEN,
  ST_CLOSE,
  ST_PRINT,
  ST_INPUTope,
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
    return ST_PRINTexpr;
  if (keyword == "INPUT")
    return ST_INPUTops;
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
    return ST_PRINTexpr;
  if (keyword == "INPUT#")
    return ST_INPUTops;
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
  if (keyword == "MAT READ")
    return ST_MATREAD;
  return ST_UNKNOWN;
}

void runInterpreter(PROGRAM_STRUCTURE &program) {
  for (const auto &entry : program.programSource) {
    auto linenum = entry.first;
    auto code2 = entry.second;
    std::string code = code2;

    {
      std::cout << "Executing line " << linenum << ": " << code << std::endl;
      // TODO: Add interpreter logic here
      std::string keyword = code.substr(code.find(" "));
      std::cout << "keyword(" << keyword << ")" << std::endl;

      StatementType stmt = identifyStatement(keyword);
      switch (stmt) {
      case ST_PRINTFILEUSING:
        executePRINTexpr(code);
      case ST_LET:
        executeLET(code);
        break;
      case ST_PRINTexpr:
        executePRINTexpr(code);
        break;
      case ST_INPUTops:
        executeINPUTops(code);
        break;
      case ST_GOTO:
        executeGOTO(code);
        break;
      case ST_IF:
        executeIF(code);
        break;
      case ST_FOR:
        executeFOR(code);
        break;
      case ST_NEXT:
        executeNEXT(code);
        break;
      case ST_READ:
        executeREAD(code);
        break;
      case ST_DATA:
        executeDATA(code);
        break;
      case ST_RESTORE:
        executeRESTORE(code);
        break;
      case ST_END:
        executeEND(code);
        break;
      case ST_DEF:
        executeDEF(code);
        break;
      case ST_DIM:
        executeDIM(code);
        break;
      case ST_REM:
        executeREM(code);
        break;
      case ST_STOP:
        executeSTOP(code);
        break;
      case ST_GOSUB:
        executeGOSUB(code);
        break;
      case ST_RETURN:
        executeRETURN(code);
        break;
      case ST_ON:
        executeON(code);
        break;
      case ST_MAT:
        executeMATops(code);
        break;
      case ST_FORMAT:
        executeFORMAT(code);
        break;
      case ST_BEEP:
        executeBEEP(code);
        break;
      case ST_OPEN:
        executeOPEN(code);
        break;
      case ST_CLOSE:
        executeCLOSE(code);
        break;
      case ST_WHILE:
        executeWHILE(code);
        break;
      case ST_WEND:
        executeWEND(code);
        break;
      case ST_REPEAT:
        executeREPEAT(code);
        break;
      case ST_UNTIL:
        executeUNTIL(code);
        break;
      case ST_SEED:
        executeSEED(code);
        break;
      case ST_MATREAD:
        executeMATREAD(code);
        break;
      default:
        std::runtime_error("Unhandled statement: " + code);
      }
    }
  }
}
