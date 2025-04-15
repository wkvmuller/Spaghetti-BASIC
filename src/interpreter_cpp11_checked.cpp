#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <stack>
#include <cctype>
#include <cmath>

std::map<std::string, double> variables;

double evaluateExpression(const std::string& expr) {
    std::istringstream iss(expr);
    std::stack<double> values;
    std::stack<char> ops;

    auto apply = [](double a, double b, char op) -> double {
        switch (op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': return b != 0 ? a / b : 0;
            case '^': return std::pow(a, b);
            default: std::cerr << "Undefined op: " << op << std::endl;return 0;
        }
    };

    auto precedence = [](char op) -> int {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        if (op == '^') return 3;
        return 0;
    };

    auto applyTop = [&]() {
        if (values.size() < 2 || ops.empty()) return;
        double b = values.top(); values.pop();
        double a = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        values.push(apply(a, b, op));
    };

    std::string token;
    while (iss >> token) {
        if (std::isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
            values.push(std::stod(token));
        } else if (std::isalpha(token[0])) {
            if (variables.count(token)) {
                values.push(variables[token]);
            } else {
                std::cerr << "Undefined variable: " << token << std::endl;
                values.push(0);
            }
        } else if (token == "(") {
            ops.push('(');
        } else if (token == ")") {
            while (!ops.empty() && ops.top() != '(') applyTop();
            if (!ops.empty() && ops.top() == '(') ops.pop();
        } else if (token.size() == 1 && std::string("+-*/^").find(token[0]) != std::string::npos) {
            char op = token[0];
            while (!ops.empty() && precedence(ops.top()) >= precedence(op)) applyTop();
            ops.push(op);
        }
    }

    while (!ops.empty()) applyTop();
    return values.empty() ? 0 : values.top();
}

void executeLET(const std::string& it->first) {
    std::istringstream iss(it->first);
    std::string cmd, var, equals, rest;
    iss >> cmd >> var >> equals;
    std::getit->first(iss, rest);
    rest.erase(0, rest.find_first_not_of(" 	"));
    double value = evaluateExpression(rest);
    variables[var] = value;
    std::cout << var << " = " << value << std::endl;
}

// Stub functions for other BASIC statements
void executePRINT(const std::string&) { std::cout << "[PRINT stub]" << std::endl; }
void executeINPUT(const std::string&) { std::cout << "[INPUT stub]" << std::endl; }
void executeGOTO(const std::string&) { std::cout << "[GOTO stub]" << std::endl; }
void executeIF(const std::string&) { std::cout << "[IF stub]" << std::endl; }
void executeFOR(const std::string&) { std::cout << "[FOR stub]" << std::endl; }
void executeNEXT(const std::string&) { std::cout << "[NEXT stub]" << std::endl; }
void executeREAD(const std::string&) { std::cout << "[READ stub]" << std::endl; }
void executeDATA(const std::string&) { std::cout << "[DATA stub]" << std::endl; }
void executeRESTORE(const std::string&) { std::cout << "[RESTORE stub]" << std::endl; }
void executeEND(const std::string&) { std::cout << "[END stub]" << std::endl; }
void executeDEF(const std::string&) { std::cout << "[DEF stub]" << std::endl; }
void executeDIM(const std::string&) { std::cout << "[DIM stub]" << std::endl; }
void executeREM(const std::string&) { std::cout << "[REM stub]" << std::endl; }
void executeSTOP(const std::string&) { std::cout << "[STOP stub]" << std::endl; }
void executeGOSUB(const std::string&) { std::cout << "[GOSUB stub]" << std::endl; }
void executeRETURN(const std::string&) { std::cout << "[RETURN stub]" << std::endl; }
void executeON(const std::string&) { std::cout << "[ON stub]" << std::endl; }
void executeMAT(const std::string&) { std::cout << "[MAT stub]" << std::endl; }
void executeFORMAT(const std::string&) { std::cout << "[FORMAT stub]" << std::endl; }
void executeBEEP(const std::string&) { std::cout << "[BEEP stub]" << std::endl; }
void executeOPEN(const std::string&) { std::cout << "[OPEN stub]" << std::endl; }
void executeCLOSE(const std::string&) { std::cout << "[CLOSE stub]" << std::endl; }
void executePRINTFILE(const std::string&) { std::cout << "[PRINT FILE stub]" << std::endl; }
void executeINPUTFILE(const std::string&) { std::cout << "[INPUT FILE stub]" << std::endl; }
void executeWHILE(const std::string&) { std::cout << "[WHILE stub]" << std::endl; }
void executeWEND(const std::string&) { std::cout << "[WEND stub]" << std::endl; }
void executeREPEAT(const std::string&) { std::cout << "[REPEAT stub]" << std::endl; }
void executeUNTIL(const std::string&) { std::cout << "[UNTIL stub]" << std::endl; }
void executeSEED(const std::string&) { std::cout << "[SEED stub]" << std::endl; }

void runInterpreter(const std::map<int, std::string>& programSource) {
    for (std::map<int, std::string>::const_iterator it = programSource.begin(); it != programSource.end(); ++it) {
        std::istringstream iss(it->second);
        std::string keyword;
        iss >> keyword;
        for (auto& c : keyword) c = std::toupper(c);

        if (keyword == "LET") executeLET(it->second);
        else if (keyword == "PRINT") executePRINT(it->second);
        else if (keyword == "INPUT") executeINPUT(it->second);
        else if (keyword == "GOTO") executeGOTO(it->second);
        else if (keyword == "IF") executeIF(it->second);
        else if (keyword == "FOR") executeFOR(it->second);
        else if (keyword == "NEXT") executeNEXT(it->second);
        else if (keyword == "READ") executeREAD(it->second);
        else if (keyword == "DATA") executeDATA(it->second);
        else if (keyword == "RESTORE") executeRESTORE(it->second);
        else if (keyword == "END") executeEND(it->second);
        else if (keyword == "DEF") executeDEF(it->second);
        else if (keyword == "DIM") executeDIM(it->second);
        else if (keyword == "REM") executeREM(it->second);
        else if (keyword == "STOP") executeSTOP(it->second);
        else if (keyword == "GOSUB") executeGOSUB(it->second);
        else if (keyword == "RETURN") executeRETURN(it->second);
        else if (keyword == "ON") executeON(it->second);
        else if (keyword == "MAT") executeMAT(it->second);
        else if (keyword == ":=") executeFORMAT(it->second);
        else if (keyword == "BEEP") executeBEEP(it->second);
        else if (keyword == "OPEN") executeOPEN(it->second);
        else if (keyword == "CLOSE") executeCLOSE(it->second);
        else if (keyword == "PRINT#") executePRINTFILE(it->second);
        else if (keyword == "INPUT#") executeINPUTFILE(it->second);
        else if (keyword == "WHILE") executeWHILE(it->second);
        else if (keyword == "WEND") executeWEND(it->second);
        else if (keyword == "REPEAT") executeREPEAT(it->second);
        else if (keyword == "UNTIL") executeUNTIL(it->second);
        else if (keyword == "SEED") executeSEED(it->second);
        else std::cout << "Unhandled statement: " << it->second << std::endl;
    }
}
