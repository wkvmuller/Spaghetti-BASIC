#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <ctime>

// Variable types
enum VariableType {
    VT_UNKNOWN,
    VT_TEXT,
    VT_INT,
    VT_DOUBLE,
    VT_STRING,
    VT_CONSTANT
};

// Information about scalar variables
struct VarInfo {
    VariableType vT;
    std::string s;
    bool isFileOpen;
    double d;
    long long ll;
};

// Global storage for variables
extern std::map<std::string, VarInfo> variables;

// Argument information for functions and DATA
struct ArgsInfo {
    long long linenumber;
    std::string identifiername;
    bool isstring;
    std::string s;
    double d;
};
ArgsInfo makeArgsInfo(long long line, std::string idname,
                      bool isstring = false, std::string str = "",
                      double d = 0.0);

// Sparse/dense array representation
struct ArrayInfo {
    std::vector<int> shape;
    std::vector<double> data;                       // numeric dense
    std::map<std::vector<int>, double> sparse;      // numeric sparse
    std::vector<std::string> dataStr;               // string dense
    std::map<std::vector<int>, std::string> stringSparse; // string sparse
};

// Global storage for arrays
extern std::map<std::string, ArrayInfo> arrays;

// Loop stack frame for FOR, WHILE, REPEAT
struct LoopFrame {
    std::string var;
    bool isString;
    char startChar;
    double final;
    double step;
    int returnLine;
};
extern std::vector<LoopFrame> loopStack;

// Stack for GOSUB/RETURN
extern std::stack<int> gosubStack;

// Current executing line number
extern long long currentline;

// Return type for expression evaluation
struct IdentifierReturn {
    bool isstring;
    std::string s;
    double d;
};

// Core expression and function evaluation
IdentifierReturn evaluateFunction(const std::string& name,
                                  const std::vector<ArgsInfo>& args);
IdentifierReturn evaluateStringFunction(const std::string& name,
                                        const std::vector<ArgsInfo>& args);
double evaluateExpression(const std::string& expr, long long currentline);

// Utility for indexed array tokens
bool parseIndexedArray(const std::string& token,
                       std::string& name,
                       std::vector<int>& indices);

// Sparse array helpers
ArgsInfo getSparseValue(const std::string& name,
                        const std::vector<int>& idx);
void setSparseValue(const std::string& name,
                    const std::vector<int>& idx,
                    const ArgsInfo& value);
void sparseTrim(ArrayInfo& matrix);
void sparseMultiplyScalar(ArrayInfo& matrix, double scalar);

// Matrix operations
bool invertMatrix(const std::vector<double>& input,
                  std::vector<double>& output,
                  int size);
double determinant(const std::vector<double>& mat, int n);

// BASIC statement handlers
void executeLET(const std::string& line);
void executePRINT(const std::string& line);
void executeINPUT(const std::string& line);
void executeGO(const std::string& line);
void executeIF(const std::string& line);
void executeFOR(const std::string& line);
void executeNEXT(const std::string& line);
void executeREAD(const std::string& line);
void executeDATA(const std::string& line);
void executeRESTORE(const std::string& line);
void executeEND(const std::string& line);
void executeDEF(const std::string& line);
void executeDIM(const std::string& line);
void executeREM(const std::string& line);
void executeSTOP(const std::string& line);
void executeGOSUB(const std::string& line);
void executeRETURN(const std::string& line);
void executeON(const std::string& line);
void executeMAT(const std::string& line);
void executeMATREAD(const std::string& line);
void executeMATPRINT(const std::string& line);
void executeMATPRINTFILE(const std::string& line);
void executePRINTFILE(const std::string& line);
void executeINPUTFILE(const std::string& line);
void executePRINTFILEUSING(const std::string& line);
void executeFORMAT(const std::string& line);
void executeBEEP(const std::string& line);
void executeOPEN(const std::string& line);
void executeCLOSE(const std::string& line);
void executeWHILE(const std::string& line);
void executeWEND(const std::string& line);
void executeREPEAT(const std::string& line);
void executeUNTIL(const std::string& line);
void executeSEED(const std::string& line);

#endif // INTERPRETER_H
