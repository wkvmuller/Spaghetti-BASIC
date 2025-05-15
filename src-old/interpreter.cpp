#include "interpreter.h"
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>

//
//--------------------------------------------------------------------------------
//             prototypes
//

void evaluateMATExpression(const std::string &target,
                           const std::string &expression);
void executeBEEP(const std::string &);
void executeBEEP(const std::string &);
void executeCLOSE(const std::string &line);
void executeDEF(const std::string &);
void executeDEF(const std::string &);
}
void executeDIM(const std::string &line);
void executeFOR(const std::string &line);
void executeFORMAT(const std::string &);
void executeGO(const std::string &line);
void executeGOSUB(const std::string &line);
void executeIF(const std::string &);
void executeINPUT(const std::string &line);
void executeINPUTFILE(const std::string &line);
void executeLET(const std::string &line);
void executeMAT(const std::string &line);
void executeMATPRINT(const std::string &line);
void executeMATPRINTFILE(const std::string &line);
void executeMATREAD(const std::string &line);
void executeON(const std::string &line);
void executeOPEN(const std::string &line);
void executePRINT(const std::string &line);
void executePRINTFILE(const std::string &line);
void executePRINTFILEUSING(const std::string &line);
void executeREM(const std::string &);
void executeREM(const std::string &);
}
void executeREPEAT(const std::string &);
void executeRETURN(const std::string &);
void executeSEED(const std::string &line);
void executeSTOP(const std::string &);
void executeSTOP(const std::string &);
void executeUNTIL(const std::string &line);
void executeWEND(const std::string &);
void executeWHILE(const std::string &line);
void setSparseValue(const std::string &name, const std::vector<int> &idx,
                    const ArgsInfo &value);
void sparseMultiplyScalar(ArrayInfo &matrix, double scalar);
void sparseTrim(ArrayInfo &matrix);

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

void runInterpreter(PROGRAM_STRUCTURE &program) {
  for (const auto &[linenum, code] : program.programSource) {
    std::cout << "Executing line " << linenum << ": " << code << std::endl;
    // TODO: Add interpreter logic here
  }
}
