#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <vector>

std::map<std::string, double> variables;

// ========================= Expression Evaluator =========================

class Parser {
public:
    Parser(const std::string& expr) : input(expr), pos(0) {}

    double parse() {
        double result = parseExpression();
        skipWhitespace();
        if (pos != input.length()) {
            throw std::runtime_error("Unexpected text after expression");
        }
        return result;
    }

private:
    std::string input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.length() && std::isspace(input[pos])) ++pos;
    }

    char peek() {
        skipWhitespace();
        return pos < input.length() ? input[pos] : '\0';
    }

    char get() {
        skipWhitespace();
        return pos < input.length() ? input[pos++] : '\0';
    }

    double parseExpression() {
        double value = parseTerm();
        while (true) {
            char op = peek();
            if (op == '+' || op == '-') {
                get();
                double rhs = parseTerm();
                value = (op == '+') ? value + rhs : value - rhs;
            } else break;
        }
        return value;
    }

    double parseTerm() {
        double value = parseFactor();
        while (true) {
            char op = peek();
            if (op == '*' || op == '/') {
                get();
                double rhs = parseFactor();
                value = (op == '*') ? value * rhs : value / rhs;
            } else break;
        }
        return value;
    }

    double parseFactor() {
        double value = parsePrimary();
        while (peek() == '^') {
            get();
            value = std::pow(value, parsePrimary());
        }
        return value;
    }

    double parsePrimary() {
        skipWhitespace();
        if (peek() == '(') {
            get();
            double val = parseExpression();
            if (get() != ')') throw std::runtime_error("Expected ')'");
            return val;
        } else if (std::isalpha(peek())) {
            std::string name = parseIdentifier();
            if (peek() == '(') {
                get();
                std::vector<double> args;
                if (peek() != ')') {
                    do {
                        args.push_back(parseExpression());
                    } while (peek() == ',' && get());
                }
                if (get() != ')') throw std::runtime_error("Expected ')' after function args");
                return evaluateFunction(name, args);
            } else {
                return variables.count(name) ? variables[name] : 0.0;
            }
        } else {
            return parseNumber();
        }
    }

    std::string parseIdentifier() {
        size_t start = pos;
        while (pos < input.length() && (std::isalnum(input[pos]) || input[pos] == '$')) ++pos;
        return input.substr(start, pos - start);
    }

    double parseNumber() {
        size_t start = pos;
        while (pos < input.length() && (std::isdigit(input[pos]) || input[pos] == '.')) ++pos;
        return std::stod(input.substr(start, pos - start));
    }

    double evaluateFunction(const std::string& name, const std::vector<double>& args) {
        if (name == "LOGX") return std::log(args[1]) / std::log(args[0]);
        if (name == "SIN") return std::sin(args[0]);
        if (name == "COS") return std::cos(args[0]);
        if (name == "TAN") return std::tan(args[0]);
        if (name == "SQR") return std::sqrt(args[0]);
        if (name == "LOG") return std::log(args[0]);
        if (name == "LOG10" || name == "CLOG") return std::log10(args[0]);
        if (name == "EXP") return std::exp(args[0]);
        if (name == "INT") return std::floor(args[0]);
        if (name == "ROUND") return std::round(args[0]);
        if (name == "FLOOR") return std::floor(args[0]);
        if (name == "CEIL") return std::ceil(args[0]);
        if (name == "POW") return std::pow(args[0], args[1]);
        throw std::runtime_error("Unknown function: " + name);
    }
};

double evaluateExpression(const std::string& expr) {
    return Parser(expr).parse();
}

// ========================= Statement Handlers =========================

void executeLET(const std::string& line) {
    std::istringstream iss(line);
    std::string keyword, var, eq;
    iss >> keyword >> var >> eq;
    std::string expr;
    std::getline(iss, expr);
    expr.erase(0, expr.find_first_not_of(" 	"));
    double value = evaluateExpression(expr);
    variables[var] = value;
    std::cout << var << " = " << value << std::endl;
}

// All other stubs
void executePRINT(const std::string&) { std::cout << "[PRINT stub]\n"; }
void executeINPUT(const std::string&) { std::cout << "[INPUT stub]\n"; }
void executeGOTO(const std::string&) { std::cout << "[GOTO stub]\n"; }
void executeIF(const std::string&) { std::cout << "[IF stub]\n"; }
void executeFOR(const std::string&) { std::cout << "[FOR stub]\n"; }
void executeNEXT(const std::string&) { std::cout << "[NEXT stub]\n"; }
void executeREAD(const std::string&) { std::cout << "[READ stub]\n"; }
void executeDATA(const std::string&) { std::cout << "[DATA stub]\n"; }
void executeRESTORE(const std::string&) {
    std::deque<std::string> restored;
    for (const auto& [line, content] : programSource) {
        std::string upper = content;
        for (char& c : upper) c = toupper(c);
        if (upper.find("DATA ") == 0) {
            std::string data = content.substr(4);
            std::stringstream ss(data);
            std::string token;
            while (std::getline(ss, token, ',')) {
                token.erase(0, token.find_first_not_of(" 	"));
                token.erase(token.find_last_not_of(" 	") + 1);
                if (!token.empty()) {
                    restored.push_back(token);
                }
            }
        }
    }
    dataPool = restored;
    std::cout << "DATA pool restored to original state." << std::endl;
}
void executeEND(const std::string&) { std::cout << "[END stub]\n"; }
void executeDEF(const std::string&) { std::cout << "[DEF stub]\n"; }
void executeDIM(const std::string&) { std::cout << "[DIM stub]\n"; }
void executeREM(const std::string&) { std::cout << "[REM stub]\n"; }
void executeSTOP(const std::string&) { std::cout << "[STOP stub]\n"; }
void executeGOSUB(const std::string&) { std::cout << "[GOSUB stub]\n"; }
void executeRETURN(const std::string&) { std::cout << "[RETURN stub]\n"; }
void executeON(const std::string&) { std::cout << "[ON stub]\n"; }
void executeMAT(const std::string&) { std::cout << "[MAT stub]\n"; }
void executeFORMAT(const std::string&) { std::cout << "[FORMAT stub]\n"; }
void executeBEEP(const std::string&) { std::cout << "[BEEP stub]\n"; }
void executeOPEN(const std::string&) { std::cout << "[OPEN stub]\n"; }
void executeCLOSE(const std::string&) { std::cout << "[CLOSE stub]\n"; }
void executePRINTFILE(const std::string&) { std::cout << "[PRINT FILE stub]\n"; }
void executeINPUTFILE(const std::string&) { std::cout << "[INPUT FILE stub]\n"; }
void executeWHILE(const std::string&) { std::cout << "[WHILE stub]\n"; }
void executeWEND(const std::string&) { std::cout << "[WEND stub]\n"; }
void executeREPEAT(const std::string&) { std::cout << "[REPEAT stub]\n"; }
void executeUNTIL(const std::string&) { std::cout << "[UNTIL stub]\n"; }
void executeSEED(const std::string&) { std::cout << "[SEED stub]\n"; }

// ========================= Dispatcher =========================

enum StatementType {
    ST_UNKNOWN, ST_LET, ST_PRINT, ST_INPUT, ST_GOTO, ST_IF, ST_FOR, ST_NEXT,
    ST_READ, ST_DATA, ST_RESTORE, ST_END, ST_DEF, ST_DIM, ST_REM, ST_STOP,
    ST_GOSUB, ST_RETURN, ST_ON, ST_MAT, ST_FORMAT, ST_BEEP, ST_OPEN, ST_CLOSE,
    ST_PRINTFILE, ST_INPUTFILE, ST_WHILE, ST_WEND, ST_REPEAT, ST_UNTIL, ST_SEED
};

StatementType identifyStatement(const std::string& keyword) {
    if (keyword == "LET") return ST_LET;
    if (keyword == "PRINT") return ST_PRINT;
    if (keyword == "INPUT") return ST_INPUT;
    if (keyword == "GOTO") return ST_GOTO;
    if (keyword == "IF") return ST_IF;
    if (keyword == "FOR") return ST_FOR;
    if (keyword == "NEXT") return ST_NEXT;
    if (keyword == "READ") return ST_READ;
    if (keyword == "DATA") return ST_DATA;
    if (keyword == "RESTORE") return ST_RESTORE;
    if (keyword == "END") return ST_END;
    if (keyword == "DEF") return ST_DEF;
    if (keyword == "DIM") return ST_DIM;
    if (keyword == "REM") return ST_REM;
    if (keyword == "STOP") return ST_STOP;
    if (keyword == "GOSUB") return ST_GOSUB;
    if (keyword == "RETURN") return ST_RETURN;
    if (keyword == "ON") return ST_ON;
    if (keyword == "MAT") return ST_MAT;
    if (keyword == ":=") return ST_FORMAT;
    if (keyword == "BEEP") return ST_BEEP;
    if (keyword == "OPEN") return ST_OPEN;
    if (keyword == "CLOSE") return ST_CLOSE;
    if (keyword == "PRINT#") return ST_PRINTFILE;
    if (keyword == "INPUT#") return ST_INPUTFILE;
    if (keyword == "WHILE") return ST_WHILE;
    if (keyword == "WEND") return ST_WEND;
    if (keyword == "REPEAT") return ST_REPEAT;
    if (keyword == "UNTIL") return ST_UNTIL;
    if (keyword == "SEED") return ST_SEED;
    return ST_UNKNOWN;
}

void runInterpreter(const std::map<int, std::string>& programSource) {
    for (std::map<int, std::string>::const_iterator it = programSource.begin(); it != programSource.end(); ++it) {
        std::istringstream iss(it->second);
        std::string keyword;
        iss >> keyword;
        for (size_t i = 0; i < keyword.length(); ++i) keyword[i] = toupper(keyword[i]);

        StatementType stmt = identifyStatement(keyword);
        switch (stmt) {
            case ST_LET: executeLET(it->second); break;
            case ST_PRINT: executePRINT(it->second); break;
            case ST_INPUT: executeINPUT(it->second); break;
            case ST_GOTO: executeGOTO(it->second); break;
            case ST_IF: executeIF(it->second); break;
            case ST_FOR: executeFOR(it->second); break;
            case ST_NEXT: executeNEXT(it->second); break;
            case ST_READ: executeREAD(it->second); break;
            case ST_DATA: executeDATA(it->second); break;
            case ST_RESTORE: executeRESTORE(it->second); break;
            case ST_END: executeEND(it->second); break;
            case ST_DEF: executeDEF(it->second); break;
            case ST_DIM: executeDIM(it->second); break;
            case ST_REM: executeREM(it->second); break;
            case ST_STOP: executeSTOP(it->second); break;
            case ST_GOSUB: executeGOSUB(it->second); break;
            case ST_RETURN: executeRETURN(it->second); break;
            case ST_ON: executeON(it->second); break;
            case ST_MAT: executeMAT(it->second); break;
            case ST_FORMAT: executeFORMAT(it->second); break;
            case ST_BEEP: executeBEEP(it->second); break;
            case ST_OPEN: executeOPEN(it->second); break;
            case ST_CLOSE: executeCLOSE(it->second); break;
            case ST_PRINTFILE: executePRINTFILE(it->second); break;
            case ST_INPUTFILE: executeINPUTFILE(it->second); break;
            case ST_WHILE: executeWHILE(it->second); break;
            case ST_WEND: executeWEND(it->second); break;
            case ST_REPEAT: executeREPEAT(it->second); break;
            case ST_UNTIL: executeUNTIL(it->second); break;
            case ST_SEED: executeSEED(it->second); break;
            default:
                std::cout << "Unhandled statement: " << it->second << std::endl;
        }
    }
}
