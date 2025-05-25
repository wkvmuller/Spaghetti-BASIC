#include "interpreter.h"
#include "program_structure.h"

extern PROGRAM_STRUCTURE program;

extern int currentLine;


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
// void executeIF(const std::string &);
// void executeMATPRINT(const std::string &line);
// void executeMATPRINTFILE(const std::string &line);
// void executeMATREAD(const std::string &line);
// Enforce matrix declaration for READ targets


// void executeON(const std::string &line);
// void executeOPEN(const std::string &line);
// void executeREM(const std::string &);
// void executeREPEAT(const std::string &);
// void executeRETURN(const std::string &);
// void executeUNTIL(const std::string &line);
// void executeWEND(const std::string &);
// void executeWHILE(const std::string &line);
/*
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
*/

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
// Enforce RESTORE only resets pointer, check related matrices during READ
// If you track named DATA segments in future, validate them here
// —————————————————————————————————————————————————————————
void executeRESTORE(const std::string & /*line*/)
 { program.dataPointer = 0; }

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
