#ifndef INTERPRETER_H
#define INTERPRETER_H


#include "program_structure.h"
#include <limits.h>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <regex>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <sstream>


extern PROGRAM_STRUCTURE program;

extern int currentLine;
//

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

struct IdentifierReturn {
  bool isstring;
  std::string s;
  double d;
};

//
//--------------------------------------------------------------------------------
//             prototypes
//

void evaluateMATExpression(const std::string &target,
                           const std::string &expression);
                           
/*void executeBEEP(const std::string &);
void executeCLOSE(const std::string &line);
void executeDEF(const std::string &);
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
void executeREM(const std::string &);
void executeREPEAT(const std::string &);
void executeRETURN(const std::string &);
void executeSEED(const std::string &line);
void executeSTOP(const std::string &);
void executeUNTIL(const std::string &line);
void executeWEND(const std::string &);
void executeWHILE(const std::string &line);
void executePRINTFILE(const std::string &line);
void executePRINTUSING(const std::string &line, std::ostream &out = std::cout);
void executePRINTFILEUSING(const std::string &line);
*/

//=========================================================
#endif // INTERPRETER_H
