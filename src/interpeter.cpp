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

//std::map<std::string, VarInfo> variables;

//
//--------------------------------------------------------------------------------
//             prototypes
//
//
// void evaluateMATExpression(const std::string &target,
//                           const std::string &expression);
extern void executeBEEP(const std::string &);
//extern void dispatchStatement(const std::string &);
// void executeCLOSE(const std::string &line);
// void executeDEF(const std::string &);
// void executeDIM(const std::string &line);
extern void executeFOR(const std::string &line);
extern void executeFORMAT(const std::string &);
extern void executeGO(const std::string &line);
extern void executeGOTO(const std::string &line);
extern void executeGOSUB(const std::string &line);
extern void executeNEXT(const std::string &line);
// void executeIF(const std::string &);
extern void executeLET(const std::string &line);
extern void executeREAD(const std::string &line);
extern void executeRESTORE(const std::string &line);
void executeMATops(const std::string &line);
// void executeMATPRINT(const std::string &line);
// void executeMATPRINTFILE(const std::string &line);
// void executeMATREAD(const std::string &line);
extern void executeON(const std::string &line);
extern void executeWEND(const std::string &line);
extern void executeUNTIL(const std::string &line);
extern void executeREPEAT(const std::string &line);
extern void executeWHILE(const std::string &);
// void executeOPEN(const std::string &line);
// void executeREM(const std::string &);
// void executeREPEAT(const std::string &);
// void executeRETURN(const std::string &);
extern void executeSEED(const std::string &line);
extern void executeSTOP(const std::string &);
extern void executeDATA(const std::string &);
extern void executeDEF(const std::string &);
extern void executeDIM(const std::string &);
extern void executeRETURN(const std::string &);

// void executeUNTIL(const std::string &line);
extern  void executeEND(const std::string &);
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


//
//=========================================================================
//  Statments support.
//

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
/*   else if (kw == "RETURN") {
          executeRETURN(stmt);
      }
      else if (kw == "ON") {
          executeON(stmt);
      }
      else if (kw == "IF") {
          executeIF(stmt);
      }
      else if (kw == "FOR") {
          executeFOR(stmt);
      }
      else if (kw == "NEXT") {
          executeNEXT(stmt);
      }
      else if (kw == "WHILE") {
          executeWHILE(stmt);
      }
      else if (kw == "WEND") {
          executeWEND(stmt);
      }
      else if (kw == "REPEAT") {
         executeREPEAT(stmt);
      }
      else if (kw == "UNTIL") {
          executeUNTIL(stmt);
      }
*/

  else if (kw == "MAT") {
    executeMATops(stmt);
  } else if (kw == "SEED") {
    executeSEED(stmt);
  } else if (kw == "STOP") {
    executeSTOP(stmt);
  } else if (kw == "END") {
    executeEND(stmt);
  }
/*      else if (kw == "FORMAT") {
          executeFORMAT(stmt);
      }
*/
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
  ST_MATops,
  ST_FORMAT,
  ST_BEEP,
  ST_OPEN,
  ST_CLOSE,
  ST_PRINT,
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
  if (keyword == "PRINT" || keyword == "PRINT#")
    return ST_PRINTexpr;
  if (keyword == "INPUT" || keyword == "INPUT#")
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
    return ST_MATops;
  if (keyword == ":=")
    return ST_FORMAT;
  if (keyword == "BEEP")
    return ST_BEEP;
  if (keyword == "OPEN")
    return ST_OPEN;
  if (keyword == "CLOSE")
    return ST_CLOSE;
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
      case ST_MATops:
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
      default:
        std::runtime_error("Unhandled statement: " + code);
      }
    }
  }
}
