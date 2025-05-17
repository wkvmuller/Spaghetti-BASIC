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
#include <vector>
*/

extern PROGRAM_STRUCTURE program;

extern int currentLine;
//
//--------------------------------------------------------------------------------
//            Global Variables, structs, etc & helper functions.
//

//
//--------------------------------------------------------------------------------
//             prototypes
//
/*
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
extern double evalExpression(const std::string &expr);
extern std::string trim(const std::string &s);
*/
// ========================= Expression Evaluator =========================
//
// Must match exactly these signatures:

// Simple file‐PRINT (no USING)
void executePRINTFILE(const std::string &line) {
    // parse the “PRINT #chan, …” channel, lookup program.fileHandles[chan],
    // then delegate to executePRINT(line, *it->second.stream);
}

// File‐PRINT USING
void executePRINTFILEUSING(const std::string &line) {
    // parse “PRINT #chan, USING”, lookup channel,
    // then delegate to executePRINTUSING(line, *it->second.stream);
}

// You should already have this in InputOutput.cpp:
void executePRINTUSING(const std::string &line, std::ostream &out) {
    // your PRINT USING implementation
}

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

void executeFORMAT(const std::string &line) {
  std::string ss = "Stub of FORMAT ";
  throw std::runtime_error(ss + line);
}

// Helper to find a line in programSource or throw
static std::map<int, std::string>::const_iterator findLine(int ln) {
  auto it = program.programSource.find(ln);
  if (it == program.programSource.end())
    throw std::runtime_error("RUNTIME ERROR: Undefined line " +
                             std::to_string(ln));
  return it;
}

// Shared helper for INPUT: read variables from an istream
static void processInputList(const std::string &vars, std::istream &in) {
    std::stringstream ss(vars);
    std::string v;
    while (std::getline(ss, v, ',')) {
        v = trim(v);
        if (v.empty()) continue;
        bool isString = (!v.empty() && v.back() == '$');
        std::string name = isString ? v.substr(0, v.size()-1) : v;
        VarInfo info;
        info.isArray = false;
        if (isString) {
            info.isString = true;
            if (!std::getline(in, info.stringValue))
                throw std::runtime_error("RUNTIME ERROR: Failed to read string for " + name);
            program.stringVariables[name] = info;
        } else {
            double d;
            if (!(in >> d))
                throw std::runtime_error("RUNTIME ERROR: Failed to read number for " + name);
            info.numericValue = d;
            info.isString = false;
            program.numericVariables[name] = info;
        }
    }
}

// Dispatcher for INPUT statements:
// 1) INPUT "prompt", varlist
// 2) INPUT #chan, varlist
// 3) INPUT varlist
void executeINPUTops(const std::string &line) {
    static const std::regex promptRe(
        R"(^\\s*INPUT\\s+\"([^\"]*)\"\\s*,\\s*(.+)$)",
        std::regex::icase
    );
    static const std::regex fileReInput(
        R"(^\\s*INPUT\\s*#\\s*(\\d+)\\s*,\\s*(.+)$)",
        std::regex::icase
    );
    static const std::regex varRe(
        R"(^\\s*INPUT\\s+(.+)$)",
        std::regex::icase
    );

    std::smatch m;
    if (std::regex_match(line, m, promptRe)) {
        std::cout << m[1].str();
        processInputList(m[2].str(), std::cin);
    }
    else if (std::regex_match(line, m, fileReInput)) {
        int chan = std::stoi(m[1].str());
        auto it = program.fileHandles.find(chan);
        if (it == program.fileHandles.end() || !it->second.stream || !it->second.stream->is_open())
            throw std::runtime_error("RUNTIME ERROR: File channel " + std::to_string(chan) + " not open");
        processInputList(m[2].str(), *it->second.stream);
    }
    else if (std::regex_match(line, m, varRe)) {
        processInputList(m[1].str(), std::cin);
    }
    else {
        throw std::runtime_error("SYNTAX ERROR: Invalid INPUT statement: " + line);
    }
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
    if (!first)
      out << ' ';
    first = false;

    // String literal?
    if (item.size() >= 2 && item.front() == '"' && item.back() == '"') {
      out << item.substr(1, item.size() - 2);
    }
    // String expression?
    else if (
        !item.empty() &&
        (item.back() == '$' ||
         std::regex_search(
             item,
             std::regex(
                 R"(\b(LEFT\$|RIGHT\$|MID\$|LEN\$|CHR\$|STRING\$|TIME\$|DATE\$)\b)",
                 std::regex::icase)))) {
      out << evalStringExpression(item);
    }
    // Numeric expression
    else {
      out << evalExpression(item);
    }
  }
  out << std::endl;
}

// Dispatcher for PRINT statements:
// 1) PRINT items to console
// 2) PRINT USING format to console
// 3) PRINT #chan, items to file
void executePRINTexpr(const std::string &line) {
 static const std::regex fileRePrint(
        R"PRINTF(^\s*PRINT\s*#\s*(\d+)\s*,)PRINTF",
        std::regex::icase
    );
    static const std::regex usingRe(
        R"USING(^\s*PRINT\s+USING\b)USING",
        std::regex::icase
    );
    static const std::regex printRe(
        R"PRINT(^\s*PRINT\b)PRINT",
        std::regex::icase
    );
    std::smatch m;
    if (std::regex_search(line, m, fileRePrint)) {
        int chan = std::stoi(m[1].str());
        auto it = program.fileHandles.find(chan);
        if (it == program.fileHandles.end() || !it->second.stream || !it->second.stream->is_open())
            throw std::runtime_error("RUNTIME ERROR: File channel " + std::to_string(chan) + " not open");
        executePRINT(line, *it->second.stream);
    }
    else if (std::regex_search(line, usingRe)) {
        executePRINTUSING(line, std::cout);
    }
    else if (std::regex_search(line, printRe)) {
        executePRINT(line, std::cout);
    }
    else {
        throw std::runtime_error("SYNTAX ERROR: Invalid PRINT statement: " + line);
    }

 // if (args.empty())
 //    throw std::runtime_error("RUNTIME ERROR: No arguments for PRINT USING");

}

