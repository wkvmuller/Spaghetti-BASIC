#include "program_structure.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <string>
extern PROGRAM_STRUCTURE program;

// Helper to trim whitespace
/*
 * static std::string trim(const std::string &s) {
  const char *WS = " \t\r\n";
  size_t start = s.find_first_not_of(WS);
  if (start == std::string::npos)
    return "";
  size_t end = s.find_last_not_of(WS);
  return s.substr(start, end - start + 1);
}
*/

// Forward-declare numeric eval for embedded numeric args
double evalExpression(const std::string &expr);

// Evaluates a string expression, supporting variables, literals, and string
// functions.
std::string evalStringExpression(const std::string &expr) {
  size_t pos = 0;
  auto skipWS = [&]() {
    while (pos < expr.size() && std::isspace(expr[pos]))
      ++pos;
  };
  skipWS();
  // String literal
  if (pos < expr.size() && expr[pos] == '"') {
    size_t start = ++pos;
    while (pos < expr.size() && expr[pos] != '"')
      ++pos;
    if (pos >= expr.size())
      throw std::runtime_error("Unterminated string literal");
    std::string lit = expr.substr(start, pos - start);
    ++pos;
    return lit;
  }
  // Identifier or function
  if (pos < expr.size() && std::isalpha(expr[pos])) {
    size_t start = pos;
    while (pos < expr.size() &&
           (std::isalnum(expr[pos]) || expr[pos] == '_' || expr[pos] == '$'))
      ++pos;
    std::string id = expr.substr(start, pos - start);
    skipWS();
    // Function call?
    if (pos < expr.size() && expr[pos] == '(') {
      ++pos;
      skipWS();
      // Parse arguments
      std::vector<std::string> args;
      size_t argStart = pos;
      int depth = 0;
      while (pos < expr.size()) {
        if (expr[pos] == '(') {
          depth++;
        } else if (expr[pos] == ')') {
          if (depth == 0)
            break;
          depth--;
        } else if (expr[pos] == ',' && depth == 0) {
          args.push_back(trim(expr.substr(argStart, pos - argStart)));
          ++pos;
          argStart = pos;
          continue;
        }
        ++pos;
      }
      args.push_back(trim(expr.substr(argStart, pos - argStart)));
      if (pos >= expr.size() || expr[pos] != ')')
        throw std::runtime_error("Missing ')' in string function call");
      ++pos;
      // Execute string function
      if (id == "LEFT$") {
        std::string s = evalStringExpression(args[0]);
        int n = static_cast<int>(evalExpression(args[1]));
        return s.substr(0, n);
      }
      if (id == "RIGHT$") {
        std::string s = evalStringExpression(args[0]);
        int n = static_cast<int>(evalExpression(args[1]));
        return s.substr(s.size() > n ? s.size() - n : 0);
      }
      if (id == "MID$") {
        std::string s = evalStringExpression(args[0]);
        int i = static_cast<int>(evalExpression(args[1])) - 1;
        int n = static_cast<int>(evalExpression(args[2]));
        if (i < 0)
          i = 0;
        if (i >= static_cast<int>(s.size()))
          return "";
        return s.substr(i, n);
      }
      if (id == "LEN$") {
        std::string s = evalStringExpression(args[0]);
        return std::to_string(s.size());
      }
      if (id == "CHR$") {
        int code = static_cast<int>(evalExpression(args[0]));
        return std::string(1, static_cast<char>(code));
      }
      if (id == "STRING$") {
        int n = static_cast<int>(evalExpression(args[0]));
        std::string fill =
            args.size() > 1 ? evalStringExpression(args[1]) : " ";
        char c = fill.empty() ? ' ' : fill[0];
        return std::string(n, c);
      }
      if (id == "TIME$") {
        std::time_t t = std::time(nullptr);
        std::tm *tm = std::localtime(&t);
        char buf[9];
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm->tm_hour,
                      tm->tm_min, tm->tm_sec);
        return std::string(buf);
      }
      if (id == "DATE$") {
        std::time_t t = std::time(nullptr);
        std::tm *tm = std::localtime(&t);
        char buf[11];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", tm->tm_year + 1900,
                      tm->tm_mon + 1, tm->tm_mday);
        return std::string(buf);
      }
      throw std::runtime_error("Unknown string function: " + id);
    }
    // Variable
    if (!id.empty() && id.back() == '$') {
      std::string name = id.substr(0, id.size() - 1);
      auto it = program.stringVariables.find(name);
      if (it != program.stringVariables.end()) {
        std::string tmpstr = it->second.stringValue;
        return tmpstr;
      }
      return "";
    }
  }
  throw std::runtime_error("Invalid string expression: " + expr);
}
