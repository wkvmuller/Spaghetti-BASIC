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
 
 
//=========================================================
#endif // INTERPRETER_H
