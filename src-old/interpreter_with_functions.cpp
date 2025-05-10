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


void evaluateMATExpression(const std::string& target, const std::string& expression);
void executeBEEP(const std::string &) { std::cout << "Stub of BEEP" << std::endl; }
void executeBEEP(const std::string &) { std::cout << "Stub of BEEP" << std::endl; }
void executeCLOSE(const std::string& line) { std::cout << "Stub of CLOSE" << std::endl; }
void executeDEF(const std::string &) { std::cout << "Stub of DEF" << std::endl; }
void exACecuteDEF(const std::string &);}
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


void runInterpreter(PROGRAM_STRUCTURE& program) {
    for (const auto& [linenum, code] : program.programSource) {
        std::cout << "Executing line " << linenum << ": " << code << std::endl;
        // TODO: Add interpreter logic here
    }
}
