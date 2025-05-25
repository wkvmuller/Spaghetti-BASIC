#include "interpreter.h"
#include "program_structure.h"
/*
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
*/

extern PROGRAM_STRUCTURE program;

extern int currentLine;

/*
 * std::map<std::string, VarInfo> variables;

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
// extern ArgsInfo makeArgsInfo(long long line, std::string idname, bool
// boolstring = false, std::string str = "", double d = 0.0);
extern void executeMATPRINT(const std::string &line,
                            std::ostream &out = std::cout);
extern void executeMATPRINTFILE(const std::string &line);
extern void executeMAT(const std::string &line);
* 
*/

extern std::map<int, std::string>::const_iterator findLine(int ln);

//
//=========================================================================
//  Statments support.
//

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
