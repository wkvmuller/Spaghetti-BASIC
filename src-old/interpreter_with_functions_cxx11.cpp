#include "program_structure.h"
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include "interpreter.h"
#include <regex>
#include <sstream>
#include <stdexcept>
#include "program_structure.h"
extern PROGRAM_STRUCTURE program;
//
//--------------------------------------------------------------------------------
//             prototypes
//

void evaluateMATExpression(const std::string& target, const std::string& expression);
void executeBEEP(const std::string &);
void executeCLOSE(const std::string& line);
void executeDEF(const std::string &);
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
void executeREPEAT(const std::string&);
void executeRETURN(const std::string &);
void executeSEED(const std::string& line);
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





//=======================================================================================
//   inline functsupport
//

IdentifierReturn evaluateFunction(const std::string &name,
                                  const std::vector<ArgsInfo> &args) {
  IdentifierReturn temp;
  temp.isstring = false;

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

  if (name == "STRING$")
    if (!args[0].isstring)
      temp.d = static_cast<double>(std::stoi(args[0].s));
    return temp;

  if (name == "LOGX") { temp.d = std::log(args[1].d) / std::log(args[0].d); return temp; }
  if (name == "SIN")  { temp.d = std::sin(args[0].d); return temp; }
  if (name == "COS")  { temp.d = std::cos(args[0].d); return temp; }
  if (name == "TAN")  { temp.d = std::tan(args[0].d); return temp; }
  if (name == "SQR")  { temp.d = std::sqrt(args[0].d); return temp; }
  if (name == "LOG")  { temp.d = std::log(args[0].d); return temp; }
  if (name == "LOG10" || name == "CLOG") { temp.d = std::log10(args[0].d); return temp; }
  if (name == "EXP")  { temp.d = std::exp(args[0].d); return temp; }
  if (name == "INT")  { temp.d = std::floor(args[0].d); return temp; }
  if (name == "ROUND"){ temp.d = std::round(args[0].d); return temp; }
  if (name == "FLOOR"){ temp.d = std::floor(args[0].d); return temp; }
  if (name == "CEIL") { temp.d = std::ceil(args[0].d); return temp; }
  if (name == "POW")  { temp.d = std::pow(args[0].d, args[1].d); return temp; }
  if (name == "RND")  { temp.d = static_cast<double>(rand()) / RAND_MAX; return temp; }
  if (name == "ASIN") { temp.d = std::asin(args[0].d); return temp; }
  if (name == "ACOS") { temp.d = std::acos(args[0].d); return temp; }
  if (name == "ATAN") { temp.d = std::atan(args[0].d); return temp; }
  if (name == "COT")  { temp.d = 1.0 / std::tan(args[0].d); return temp; }
  if (name == "SEC")  { temp.d = 1.0 / std::cos(args[0].d); return temp; }
  if (name == "CSC")  { temp.d = 1.0 / std::sin(args[0].d); return temp; }
  if (name == "DEG2RAD") { temp.d = args[0].d * M_PI / 180.0; return temp; }
  if (name == "RAD2DEG") { temp.d = args[0].d * 180.0 / M_PI; return temp; }
  if (name == "DET") {
    std::cerr << "DET() not implemented - placeholder only." << std::endl;
    temp.d = 0.0;
    return temp;
  }

  std::cerr << "Unknown function: " << name << std::endl;
  temp.d = 0.0;
  return temp;
}

IdentifierReturn evaluateStringFunction(const std::string &name,
                                        const std::vector<ArgsInfo> &args) {
  IdentifierReturn temp;
  temp.isstring = true;

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
    if (c < 0 || c > 255) { temp.s = ""; return temp; }
    temp.s = std::string(1, static_cast<char>(c));
    return temp;
  }

  if (name == "LEFT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0) n = 0;
    if (n > static_cast<int>(args[0].s.length())) n = args[0].s.length();
    temp.s = args[0].s.substr(0, n);
    return temp;
  }

  if (name == "RIGHT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0) n = 0;
    if (n > static_cast<int>(args[0].s.length())) n = args[0].s.length();
    temp.s = args[0].s.substr(args[0].s.length() - n);
    return temp;
  }

  if (name == "MID$") {
    int start = static_cast<int>(args[1].d);
    int len = static_cast<int>(args[2].d);
    if (start < 1) start = 1;
    if (len < 0) len = 0;
    if (start > static_cast<int>(args[0].s.length())) start = args[0].s.length();
    if (start - 1 + len > static_cast<int>(args[0].s.length()))
        len = args[0].s.length() - (start - 1);
    temp.s = args[0].s.substr(start - 1, len);
    return temp;
  }

  std::cerr << "ERROR: Unknown string function " << name << std::endl;
  temp.s = "";
  return temp;
}

//
//=========================================================================
//  Statments support.
//


// Trim helper (if you don’t already have it)
static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t");
    size_t b = s.find_last_not_of(" \t");
    return (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

void executeREAD(const std::string &line) {
    // Match: READ <var>[$],<var>...  (names A–Z0–9_ up to 32 chars, optional $)
    static const std::regex rgx(
        R"(^\s*READ\s+([A-Z][A-Z0-9_]{0,31}\$?(?:\s*,\s*[A-Z][A-Z0-9_]{0,31}\$?)*)\s*$)",
        std::regex::icase
    );
    std::smatch m;
    if (!std::regex_match(line, m, rgx)) {
        throw std::runtime_error("SYNTAX ERROR: Invalid READ syntax: " + line);
    }

    std::string varList = m[1].str();
    std::stringstream ss(varList);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = trim(token);
        bool wantsString = (!token.empty() && token.back() == '$');
        std::string name = wantsString ? token.substr(0, token.size()-1) : token;

        // Check for DATA exhaustion
        if (program.dataPointer >= program.dataValues.size()) {
            throw std::runtime_error("RUNTIME ERROR: No more DATA available");
        }

        // Fetch next data item
        VarInfo val = program.dataValues[program.dataPointer++];
        
        if (wantsString) {
            // Assign to string variable
            if (val.isString) {
                program.stringVariables[name] = val.stringValue;
            } else {
                program.stringVariables[name] = std::to_string(val.numericValue);
            }
        } else {
            // Assign to numeric variable
            if (!val.isString) {
                program.numericVariables[name] = val.numericValue;
            } else {
                // convert string → double
                program.numericVariables[name] = std::stod(val.stringValue);
            }
        }
    }
}

void evaluateMATExpression(const std::string& target, const std::string& expression);
void executeBEEP(const std::string &) { std::cout << "Stub of BEEP" << std::endl; }
void executeCLOSE(const std::string& line) { std::cout << "Stub of CLOSE" << std::endl; }
void executeDEF(const std::string &) { std::cout << "Stub of DEF" << std::endl; }
void exACecuteDEF(const std::string &);}

// Assumes you have a helper to eval an arithmetic expression to an int:
int evalIntExpr(const std::string &expr)#include <string>
#include <cctype>
#include <stdexcept>

// Evaluates expr and returns its integer value.
// Supports +, -, *, / and parentheses. Throws on syntax error.
int evalIntExpr(const std::string &expr) {
    size_t pos = 0;

    // Skip whitespace
    auto skipWS = [&]() {
        while (pos < expr.size() && isspace(expr[pos])) ++pos;
    };

    // Forward declarations
    std::function<long()> parseExpr, parseTerm, parseFactor;

    // <expression> ::= <term> { (+|-) <term> }
    parseExpr = [&]() -> long {
        long value = parseTerm();
        skipWS();
        while (pos < expr.size()) {
            if (expr[pos] == '+') {
                ++pos; skipWS();
                value += parseTerm();
            }
            else if (expr[pos] == '-') {
                ++pos; skipWS();
                value -= parseTerm();
            }
            else break;
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
                ++pos; skipWS();
                value *= parseFactor();
            }
            else if (expr[pos] == '/') {
                ++pos; skipWS();
                long rhs = parseFactor();
                if (rhs == 0) throw std::runtime_error("Division by zero");
                value /= rhs;
            }
            else break;
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
            ++pos; skipWS();
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
      R"(^\s*([A-Z][A-Z0-9_]{0,31}\$?)\s*\(([^)]*)\)\s*$)",
      std::regex::icase
    );

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
        throw std::runtime_error(
          "SYNTAX ERROR: DIM exceeds 15 dimensions: " + std::to_string(dims.size())
        );
    }

    // Compute total elements
    size_t totalElements = 1;
    for (int d : dims) totalElements *= d;

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

void executeFOR(const std::string &line) { std::cout << "Stub of FOR" << std::endl; }
void executeFORMAT(const std::string &) { std::cout << "Stub of FORMAT" << std::endl; }
void executeGO(const std::string &line) { std::cout << "Stub of GO" << std::endl; }
void executeGOSUB(const std::string &line) { std::cout << "Stub of GOSUB" << std::endl; }
void executeIF(const std::string &) { std::cout << "Stub of IF" << std::endl; }

// Helper to trim whitespace
static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t");
    size_t b = s.find_last_not_of(" \t");
    return (a==std::string::npos) ? "" : s.substr(a, b-a+1);
}

void executeINPUT(const std::string &line) {
    // Grammar (BNF): 
    //   <inputstmt> ::= INPUT [ <string> ; ] <varlist>       :contentReference[oaicite:0]{index=0}:contentReference[oaicite:1]{index=1}
    //   <inputfilestmt> ::= INPUT # <filenumber> , <varlist> :contentReference[oaicite:2]{index=2}:contentReference[oaicite:3]{index=3}
    static const std::regex rgx(
        R"(^\s*INPUT\s*(?:\"([^\"]*)\"\s*;\s*)?(?:#(\d+)\s*,\s*)?([A-Z][A-Z0-9_]{0,31}\$?(?:\s*,\s*[A-Z][A-Z0-9_]{0,31}\$?)*)\s*$)",
        std::regex::icase
    );

    std::smatch m;
    if (!std::regex_match(line, m, rgx)) {
        throw std::runtime_error("SYNTAX ERROR: Invalid INPUT syntax: " + line);
    }

    // Extract components
    std::string prompt    = m[1].matched ? m[1].str() : "";
    std::string chanStr   = m[2].matched ? m[2].str() : "";
    std::string varList   = m[3].str();

    // Select input stream
    std::istream *in = &std::cin;
    std::ifstream fin;
    if (!chanStr.empty()) {
        int chan = std::stoi(chanStr);
        auto it = program.fileHandles.find(chan);
        if (it == program.fileHandles.end() || !it->second.stream) {
            throw std::runtime_error("RUNTIME ERROR: File channel not open: " + chanStr);
        }
        in = it->second.stream;
    }

    // Display prompt for console INPUT
    if (!prompt.empty() && in == &std::cin) {
        std::cout << prompt;
    }

    // Parse and read each variable
    std::stringstream ss(varList);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = trim(token);
        bool isString = (!token.empty() && token.back() == '$');
        std::string name = isString ? token.substr(0, token.size()-1) : token;

        if (isString) {
            std::string tmp;
            *in >> tmp;
            program.stringVariables[name] = tmp;
        } else {
            double val;
            *in >> val;
            program.numericVariables[name] = val;
        }
    }
}

// DATA <datum>{,datum}  where datum is "quoted text" OR integer/float
void executeDATA(const std::string &line) {
    static const std::regex rgx(R"(^\s*DATA\s+(.*)\s*$)", std::regex::icase);
    std::smatch m;
    if (!std::regex_match(line, m, rgx)) {
        throw std::runtime_error("SYNTAX ERROR: Invalid DATA syntax: " + line);
    }

    std::string list = m[1].str();
    std::stringstream ss(list);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = trim(token);
        VarInfo v;
        // string literal?
        if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
            v.isString    = true;
            v.stringValue = token.substr(1, token.size() - 2);
        } else {
            // numeric (int or float)
            v.isString     = false;
            v.numericValue = std::stod(token);
        }
        v.isArray = false;
        program.dataValues.push_back(v);
    }
}

// RESTORE resets DATA pointer to start
void executeRESTORE(const std::string &line) {
    // we ignore any argument for now:
    program.dataPointer = 0;
}

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
void executeREM(const std::string &) {  }
void executeREPEAT(const std::string&) { std::cout << "Stub of REPEAT" << std::endl; }
void executeRETURN(const std::string &) { std::cout << "Stub of RETURN" << std::endl; }
void executeSEED(const std::string& line) { std::cout << "Stub of SEED" << std::endl; }
void executeSTOP(const std::string &) { std::cout << "Stub of STOP" << std::endl; }
void executeUNTIL(const std::string& line) { std::cout << "Stub of UNTIL" << std::endl; }
void executeWEND(const std::string&) { std::cout << "Stub of WEND" << std::endl; }
void executeWHILE(const std::string& line) { std::cout << "Stub of WHILE" << std::endl; }


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


void runInterpreter(PROGRAM_STRUCTURE& program) {
    for (const auto& entry : program.programSource) {
    auto linenum = entry.first;
    auto code = entry.second; {
        std::cout << "Executing line " << linenum << ": " << code << std::endl;
        // TODO: Add interpreter logic here
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
    }
}
