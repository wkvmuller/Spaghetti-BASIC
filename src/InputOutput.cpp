#include <regex>
#include <stdexcept>
#include "program_structure.h"
#include "interpreter.h"
#include "program_structure.h"
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
#include <iomanip>
#include <stdexcept>
#include <vector>
#include "program_structure.h"

extern PROGRAM_STRUCTURE program;

extern int currentLine;
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
/*
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
*/

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

//
//--------------------------------------------------------------------------------
//             prototypes
//

extern double evalExpression(const std::string &expr);
void executeCLOSE(const std::string &line);
void executeFORMAT(const std::string &);
void executeINPUT(const std::string &line);
void executeINPUTFILE(const std::string &line);
void executeINPUTSTRING(const std::string &line);
void executeOPEN(const std::string &line);
void executePRINT(const std::string &line);
void executePRINTFILE(const std::string &line);
void executePRINTFILEUSING(const std::string &line);


// ========================= Expression Evaluator =========================
//

//
//=========================================================================
//  Statments support.
//

// extern PROGRAM_STRUCTURE program;
// OPEN statement: OPEN "filename" FOR INPUT|OUTPUT|APPEND AS #<channel>
void executeOPEN(const std::string &line) {
  static const std::regex rgx("^\\s*OPEN\\s*\"([^\"]+)\"\\s+FOR\\s+(INPUT|"
                              "OUTPUT|APPEND)\\s+AS\\s*#\\s*(\\d+)\\s*$",
                              std::regex::icase);

  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid OPEN: " + line);

  std::string path = m[1].str();
  std::string mode = m[2].str();
  int chan = std::stoi(m[3].str());

  std::ios_base::openmode om;
  if (mode == "INPUT")
    om = std::ios::in;
  else if (mode == "OUTPUT")
    om = std::ios::out | std::ios::trunc;
  else
    om = std::ios::out | std::ios::app;

  // Create or overwrite the stream on that channel
  auto &fh = program.fileHandles[chan];
  fh.stream.reset(new std::fstream(path, om));
  if (!fh.stream->is_open())
    throw std::runtime_error("RUNTIME ERROR: Cannot open file: " + path);
}

// statement: CLOSE #<channel>
void executeCLOSE(const std::string &line) {
  static const std::regex rgx(R"(^\s*CLOSE\s*#\s*(\d+)\s*$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid CLOSE: " + line);

  int chan = std::stoi(m[1].str());
  auto it = program.fileHandles.find(chan);
  if (it == program.fileHandles.end() || !it->second.stream)
    throw std::runtime_error("RUNTIME ERROR: File not open on channel " +
                             std::to_string(chan));

  it->second.stream->close();
  program.fileHandles.erase(it);
}

void executeFORMAT(const std::string &) {
  std::cout << "Stub of FORMAT" << std::endl;
}

void executePRINTexpr(const std::string &line) {
  // Skip past the “PRINT” keyword
  std::istringstream iss(line);
  std::string kw;
  iss >> kw; // eats "PRINT"

  // Peek at the next non-whitespace character
  char c = iss.peek();
  if (c == '#') {
    // It’s the file-output form.  Pass the full line through.
    // executePRINTFILEUSING could be chosen here if you detect “USING” later.
    executePRINTFILE(line);
  } else {
    iss >> kw;
    if (kw == "USING") {
      executePRINTFILEUSING(line);
    } else {
      // Normal console PRINT
      executePRINT(line);
    }
  }
}

// Forward decls (you should have these elsewhere or adapt)
double evalExpression(const std::string &expr);
static std::string trim(const std::string &s);

// PRINT handler
void executePRINT(const std::string &line) {
  // Match everything after PRINT
  static const std::regex rgx(R"(^\s*PRINT\s+(.*)$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid PRINT: " + line);
  }

  std::string list = m[1].str();
  std::stringstream ss(list);
  std::string item;
  bool first = true;

  while (std::getline(ss, item, ',')) {
    item = trim(item);
    if (!first) {
      std::cout << ' ';
    }
    first = false;

    // String literal?
    if (item.size() >= 2 && item.front() == '"' && item.back() == '"') {
      std::cout << item.substr(1, item.size() - 2);
    } else {
      // Numeric expression
      double val = evalExpression(item);
      // You can control formatting here (fixed, precision, etc.)
      std::cout << val;
    }
  }

  std::cout << std::endl;
}

// Example trim helper
static std::string trim(const std::string &s) {
  const char *WS = " \t\r\n";
  size_t start = s.find_first_not_of(WS);
  if (start == std::string::npos)
    return "";
  size_t end = s.find_last_not_of(WS);
  return s.substr(start, end - start + 1);
}


// Helper to find a line in programSource or throw
static std::map<int, std::string>::const_iterator findLine(int ln) {
  auto it = program.programSource.find(ln);
  if (it == program.programSource.end())
    throw std::runtime_error("RUNTIME ERROR: Undefined line " +
                             std::to_string(ln));
  return it;
}


// Helper to trim whitespace
static std::string trim(const std::string &s) {
    const char *WS = " \t\r\n";
    size_t b = s.find_first_not_of(WS);
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(WS);
    return s.substr(b, e - b + 1);
}

// Common routine to read variables from an input stream
static void processInputList(const std::string &varList, std::istream &in) {
    std::stringstream ss(varList);
    std::string var;
    while (std::getline(ss, var, ',')) {
        var = trim(var);
        if (var.empty()) continue;
        bool isString = (var.back() == '$');
        std::string name = isString ? var.substr(0, var.size() - 1) : var;
        VarInfo v;
        v.isArray = false;
        if (isString) {
            v.isString = true;
            if (!std::getline(in, v.stringValue))
                throw std::runtime_error("RUNTIME ERROR: Failed to read string for " + name);
            program.stringVariables[name] = v;
        } else {
            v.isString = false;
            double num;
            if (!(in >> num))
                throw std::runtime_error("RUNTIME ERROR: Failed to read number for " + name);
            v.numericValue = num;
            program.numericVariables[name] = v;
        }
    }
}

// Dispatcher for INPUT operations: handles three forms:
// 1) INPUT "prompt", var1,var2$,...
// 2) INPUT #chan, var1,var2$,...
// 3) INPUT var1,var2$,...
void executeINPUTops(const std::string &line) {
    static const std::regex promptRe(R"(^\s*INPUT\s+"([^"]*)"\s*,\s*(.+)$)", std::regex::icase);
    static const std::regex fileRe  (R"(^\s*INPUT\s*#\s*(\d+)\s*,\s*(.+)$)",   std::regex::icase);
    static const std::regex varRe   (R"(^\s*INPUT\s+(.+)$)",                         std::regex::icase);

    std::smatch m;
    if (std::regex_match(line, m, promptRe)) {
        // Case 1: prompt then variables
        std::cout << m[1].str();
        processInputList(m[2].str(), std::cin);
    }
    else if (std::regex_match(line, m, fileRe)) {
        // Case 2: file-based input
        int chan = std::stoi(m[1].str());
        auto it = program.fileHandles.find(chan);
        if (it == program.fileHandles.end() || !it->second.stream || !it->second.stream->is_open())
            throw std::runtime_error("RUNTIME ERROR: File channel " + std::to_string(chan) + " not open");
        processInputList(m[2].str(), *it->second.stream);
    }
    else if (std::regex_match(line, m, varRe)) {
        // Case 3: console input without prompt
        processInputList(m[1].str(), std::cin);
    }
    else {
        throw std::runtime_error("SYNTAX ERROR: Invalid INPUT statement: " + line);
    }
}



// Trims whitespace from both ends
static std::string trim(const std::string &s) {
    const char *WS = " \t\r\n";
    size_t b = s.find_first_not_of(WS);
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(WS);
    return s.substr(b, e - b + 1);
}

/**
 * Plain PRINT:
 *   PRINT <item1>,<item2$>,...
 * Writes to the given output stream.
 */
void executePRINT(const std::string &line, std::ostream &out) {
    static const std::regex rgx(R"(^\s*PRINT\s+(.*)$)", std::regex::icase);
    std::smatch m;
    if (!std::regex_match(line, m, rgx))
        throw std::runtime_error("SYNTAX ERROR: Invalid PRINT: " + line);

    std::string list = m[1].str();
    std::stringstream ss(list);
    std::string item;
    bool first = true;

    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!first) out << ' ';
        first = false;

        // String literal?
        if (item.size() >= 2 && item.front() == '"' && item.back() == '"') {
            out << item.substr(1, item.size() - 2);
        }
        // String expression?
        else if (!item.empty() && (item.back() == '$' ||
                 std::regex_search(item, std::regex(R"(\b(LEFT\$|RIGHT\$|MID\$|LEN\$|CHR\$|STRING\$|TIME\$|DATE\$)\b)", std::regex::icase))))
        {
            out << evalStringExpression(item);
        }
        // Numeric expression
        else {
            out << evalExpression(item);
        }
    }
    out << std::endl;
}


/**
 * PRINT USING:
 *   PRINT USING <fmtLine> <item1>,<item2$>,...
 *   (also supports leading "PRINT #<chan>, USING ..." if you route here)
 */
void executePRINTUSING(const std::string &line, std::ostream &out) {
    static const std::regex rgx(
        R"(^\s*PRINT(?:\s*#\s*\d+\s*,)?\s+USING\s+(\d+)\s+(.*)\s*$)",
        std::regex::icase
    );
    std::smatch m;
    if (!std::regex_match(line, m, rgx))
        throw std::runtime_error("SYNTAX ERROR: Invalid PRINT USING: " + line);

    // Fetch format spec
    int fmtLine = std::stoi(m[1].str());
    auto it = program.printUsingFormats.find(fmtLine);
    if (it == program.printUsingFormats.end())
        throw std::runtime_error("RUNTIME ERROR: FORMAT " + std::to_string(fmtLine) + " not defined");
    const std::string spec = it->second;

    // Split argument list
    std::vector<std::string> args;
    {
        std::stringstream vs(m[2].str());
        std::string tok;
        while (std::getline(vs, tok, ',')) {
            args.push_back(trim(tok));
        }
    }
    if (args.empty())
        throw std::runtime_error("RUNTIME ERROR: No arguments for PRINT USING");

    // Walk spec, one run per arg
    size_t argIdx = 0;
    for (size_t i = 0; i < spec.size(); ) {
        char ch = spec[i];
        if (ch=='#' || ch=='l'||ch=='L' || ch=='c'||ch=='C' || ch=='r'||ch=='R') {
            char type = std::toupper(ch);
            size_t start = i;
            while (i<spec.size() && std::toupper(spec[i])==type) ++i;
            int width = int(i-start);

            if (argIdx >= args.size())
                throw std::runtime_error("RUNTIME ERROR: Not enough arguments for PRINT USING");

            const std::string &expr = args[argIdx++];
            if (type=='#') {
                // numeric field: count decimals
                std::string fld = spec.substr(start, width);
                int prec = 0;
                auto dp = fld.find('.');
                if (dp!=std::string::npos) prec = int(fld.size()-dp-1);
                double num = evalExpression(expr);
                std::ostringstream oss;
                if (prec>0) oss << std::fixed << std::setprecision(prec);
                oss << num;
                std::string outVal = oss.str();
                // right-justify
                if ((int)outVal.size()<width) out << std::string(width-outVal.size(),' ');
                out << outVal;
            } else {
                // string field
                std::string s = evalStringExpression(expr);
                if ((int)s.size()>width) s = s.substr(0,width);
                int pad = width - int(s.size());
                if (type=='L')       out << s << std::string(pad,' ');
                else if (type=='R')  out << std::string(pad,' ') << s;
                else { // center
                    int left = pad/2, right = pad-left;
                    out << std::string(left,' ') << s << std::string(right,' ');
                }
            }
        } else {
            // literal char
            out << ch;
            ++i;
        }
    }
    out << std::endl;
}

executePRINTexprconst std::string &line){void executePRINTops(const std::string &line) {
    static const std::regex fileRe(
        R"(^\s*PRINT\s*#\s*(\d+)\s*,)", std::regex::icase);
    static const std::regex usingRe(
        R"(^\s*PRINT\s+USING\b)",       std::regex::icase);
    static const std::regex printRe(
        R"(^\s*PRINT\b)",               std::regex::icase);

    std::smatch m;
    // 3) File-based simple PRINT
    if (std::regex_search(line, m, fileRe)) {
        int chan = std::stoi(m[1].str());
        auto it = program.fileHandles.find(chan);
        if (it == program.fileHandles.end() || !it->second.stream || !it->second.stream->is_open())
            throw std::runtime_error("RUNTIME ERROR: File channel " + std::to_string(chan) + " not open");
        // Delegate to generic PRINT→ostream
        executePRINT(line, *it->second.stream);
    }
    // 2) Console PRINT USING
    else if (std::regex_search(line, usingRe)) {
        executePRINTUSING(line, std::cout);
    }
    // 1) Plain console PRINT
    else if (std::regex_search(line, printRe)) {
        executePRINT(line, std::cout);
    }
    else {
        throw std::runtime_error("SYNTAX ERROR: Invalid PRINT statement: " + line);
    }
}
}
