#include <cmath>
#include <cstdlib>
#include <cctype>
#include <stdexcept>
#include <functional>
#include <string>
#include <vector>
#include "program_structure.h"
extern PROGRAM_STRUCTURE program;

// Evaluates a BASIC expression and returns its value as double.
// Supports variables, numeric literals (with optional exponent), parentheses,
// +, -, *, /, and built-in math functions.
double evalExpression(const std::string &expr) {
    size_t pos = 0;
    auto skipWS = [&]() {
        while (pos < expr.size() && std::isspace(expr[pos])) ++pos;
    };

    std::function<double()> parseExpr, parseTerm, parseFactor, parsePrimary;

    // <expression> ::= <term> { (+|-) <term> }
    parseExpr = [&]() -> double {
        double value = parseTerm();
        skipWS();
        while (pos < expr.size()) {
            if (expr[pos] == '+') {
                ++pos; skipWS();
                value += parseTerm();
            } else if (expr[pos] == '-') {
                ++pos; skipWS();
                value -= parseTerm();
            } else break;
            skipWS();
        }
        return value;
    };

    // <term> ::= <factor> { (*|/) <factor> }
    parseTerm = [&]() -> double {
        double value = parseFactor();
        skipWS();
        while (pos < expr.size()) {
            if (expr[pos] == '*') {
                ++pos; skipWS();
                value *= parseFactor();
            } else if (expr[pos] == '/') {
                ++pos; skipWS();
                double rhs = parseFactor();
                if (rhs == 0.0) throw std::runtime_error("Division by zero");
                value /= rhs;
            } else break;
            skipWS();
        }
        return value;
    };

    // <factor> ::= [-] ( <number> | <identifier> | <identifier>(<args>) | '(' <expression> ')' )
    parseFactor = [&]() -> double {
        skipWS();
        bool neg = false;
        if (pos < expr.size() && expr[pos] == '-') {
            neg = true;
            ++pos; skipWS();
        }

        double value = 0.0;
        if (pos < expr.size() && expr[pos] == '(') {
            ++pos; skipWS();
            value = parseExpr();
            skipWS();
            if (pos >= expr.size() || expr[pos] != ')')
                throw std::runtime_error("Missing closing parenthesis");
            ++pos;
        }
        // Numeric literal with optional exponent
        else if (pos < expr.size() && (std::isdigit(expr[pos]) || expr[pos] == '.')) {
            size_t start = pos;
            // Integral and fractional part
            while (pos < expr.size() && (std::isdigit(expr[pos]) || expr[pos] == '.')) ++pos;
            // Exponent part
            if (pos < expr.size() && (expr[pos] == 'e' || expr[pos] == 'E')) {
                ++pos;
                if (pos < expr.size() && (expr[pos] == '+' || expr[pos] == '-')) ++pos;
                while (pos < expr.size() && std::isdigit(expr[pos])) ++pos;
            }
            value = std::stod(expr.substr(start, pos - start));
        }
        else {
            value = parsePrimary();
        }

        return neg ? -value : value;
    };

    // Parses identifiers, function calls, and variables
    parsePrimary = [&]() -> double {
        skipWS();
        if (pos >= expr.size() || !std::isalpha(expr[pos]))
            throw std::runtime_error("Unexpected character in expression");

        // Read identifier
        size_t start = pos;
        while (pos < expr.size() && (std::isalnum(expr[pos]) || expr[pos] == '_')) ++pos;
        std::string id = expr.substr(start, pos - start);
        std::string idUp = id;
        for (char &c : idUp) c = std::toupper(c);

        skipWS();
        // Function call
        if (pos < expr.size() && expr[pos] == '(') {
            ++pos; skipWS();
            std::vector<double> args;
            if (pos < expr.size() && expr[pos] != ')') {
                do {
                    args.push_back(parseExpr());
                    skipWS();
                } while (pos < expr.size() && expr[pos] == ',' && (++pos, skipWS(), true));
            }
            if (pos >= expr.size() || expr[pos] != ')')
                throw std::runtime_error("Missing closing parenthesis in call to " + id);
            ++pos;
            // Built-in math functions
            if (idUp == "SIN")      return std::sin(args[0]);
            if (idUp == "COS")      return std::cos(args[0]);
            if (idUp == "TAN")      return std::tan(args[0]);
            if (idUp == "ATN")      return std::atan(args[0]);
            if (idUp == "ASN")      return std::asin(args[0]);
            if (idUp == "ACS")      return std::acos(args[0]);
            if (idUp == "COT")      return 1.0 / std::tan(args[0]);
            if (idUp == "SEC")      return 1.0 / std::cos(args[0]);
            if (idUp == "CSC")      return 1.0 / std::sin(args[0]);
            if (idUp == "SQR")      return std::sqrt(args[0]);
            if (idUp == "EXP")      return std::exp(args[0]);
            if (idUp == "LOG10")    return std::log10(args[0]);
            if (idUp == "LOGX")     return std::log(args[1]) / std::log(args[0]);
            if (idUp == "CLOG")     return std::log(args[0]);
            if (idUp == "INT")      return std::floor(args[0]);
            if (idUp == "ROUND")    return std::floor(args[0] + 0.5);
            if (idUp == "FLOOR")    return std::floor(args[0]);
            if (idUp == "CEIL")     return std::ceil(args[0]);
            if (idUp == "POW")      return std::pow(args[0], args[1]);
            if (idUp == "RND")      return std::rand() / (double)RAND_MAX;
            if (idUp == "DEG2RAD")  return args[0] * M_PI / 180.0;
            if (idUp == "RAD2DEG")  return args[0] * 180.0 / M_PI;
            throw std::runtime_error("Unknown function: " + id);
        }

        // Variable lookup
        auto itNum = program.numericVariables.find(id);
        if (itNum != program.numericVariables.end()) {
            return itNum->second.numericValue;
        }
        auto itStr = program.stringVariables.find(id);
        if (itStr != program.stringVariables.end()) {
            return std::stod(itStr->second.stringValue);
        }

        throw std::runtime_error("Unknown identifier: " + id);
    };

    double result = parseExpr();
    skipWS();
    if (pos != expr.size())
        throw std::runtime_error("Unexpected trailing characters in expression");
    return result;
}
