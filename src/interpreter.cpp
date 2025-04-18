#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <vector>

std::map<std::string, double> variables;

struct ArrayInfo {
    std::vector<int> shape;
    std::vector<double> data; // for <= 3D
    std::map<std::vector<int>, double> sparse; // for >= 4D
};

std::map<std::string, ArrayInfo> arrays;

struct LoopFrame {
    std::string var;
    double final;
    double step;
    int returnLine;
};

std::vector<LoopFrame> loopStack;

std::stack<int> gosubStack;


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
    std::string keyword, target, eq;
    iss >> keyword >> target >> eq;
    std::string expr;
    std::getline(iss, expr);
    expr.erase(0, expr.find_first_not_of(" \t"));

    size_t paren_pos = target.find('(');
    if (paren_pos != std::string::npos) {
        std::string var = target.substr(0, paren_pos);
        std::string subs = target.substr(paren_pos + 1);
        if (!subs.empty() && subs.back() == ')') subs.pop_back();
        std::stringstream ss(subs);
        std::string token;
        std::vector<int> indices;
        while (std::getline(ss, token, ',')) {
            indices.push_back(std::stoi(token));
        }
        double value = evaluateExpression(expr);
        if (arrays.count(var)) {
            ArrayInfo& arr = arrays[var];
            if (indices.size() != arr.shape.size()) {
                std::cerr << "ERROR: Index count mismatch for array " << var << std::endl;
                return;
            }
            if (arr.data.size()) {
                int flat = 0, stride = 1;
                for (int i = indices.size() - 1; i >= 0; --i) {
                    if (indices[i] >= arr.shape[i]) {
                        std::cerr << "ERROR: Index out of bounds in " << var << std::endl;
                        return;
                    }
                    flat += indices[i] * stride;
                    stride *= arr.shape[i];
                }
                arr.data[flat] = value;
            } else {
                arr.sparse[indices] = value;
            }
        } else {
            std::cerr << "ERROR: Undeclared array " << var << std::endl;
        }
    } else {
        double value = evaluateExpression(expr);
        variables[target] = value;
        std::cout << target << " = " << value << std::endl;
    }
}

void executePRINT(const std::string& line) {
    std::string rest = line.substr(5); // after "PRINT"
    std::stringstream ss(rest);
    std::string token;
    bool first = true;
    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (!first) std::cout << " ";
        if (!token.empty()) {
            if (token.front() == '\"' && token.back() == '\"') {
                std::cout << token.substr(1, token.length() - 2);
            } else {
                size_t paren = token.find('(');
                if (paren != std::string::npos && token.back() == ')') {
                    std::string name = token.substr(0, paren);
                    std::string index_str = token.substr(paren + 1, token.size() - paren - 2);
                    std::stringstream idxs(index_str);
                    std::string n;
                    std::vector<int> indices;
                    while (std::getline(idxs, n, ',')) indices.push_back(std::stoi(n));
                    if (arrays.count(name)) {
                        ArrayInfo& arr = arrays[name];
                        if (indices.size() != arr.shape.size()) {
                            std::cerr << "[?]";
                        } else if (!arr.data.empty()) {
                            int flat = 0, stride = 1;
                            for (int i = indices.size() - 1; i >= 0; --i) {
                                flat += indices[i] * stride;
                                stride *= arr.shape[i];
                            }
                            std::cout << arr.data[flat];
                        } else {
                            std::cout << arr.sparse[indices];
                        }
                    } else {
                        std::cerr << "[ERR]";
                    }
                } else {
                    try {
                        std::cout << evaluateExpression(token);
                    } catch (...) {
                        std::cerr << "[ERR]";
                    }
                }
            }
            first = false;
        }
    }
    std::cout << std::endl;
}

void executeINPUT(const std::string& line) {
    std::string rest = line.substr(5); // after "INPUT"
    std::stringstream ss(rest);
    std::string token;
    std::vector<std::string> variables;
    bool promptShown = false;

    // Handle optional prompt string
    if (!rest.empty() && rest[0] == '"') {
        size_t endQuote = rest.find('"', 1);
        if (endQuote != std::string::npos) {
            std::string prompt = rest.substr(1, endQuote - 1);
            std::cout << prompt << " ";
            promptShown = true;
            rest = rest.substr(endQuote + 1);
            size_t semi = rest.find(';');
            if (semi != std::string::npos) rest = rest.substr(semi + 1);
        }
    }

    ss.clear(); ss.str(rest);
    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" 	"));
        token.erase(token.find_last_not_of(" 	") + 1);
        if (!token.empty()) {
            variables.push_back(token);
        }
    }

    for (const auto& var : variables) {
        std::cout << var << "? ";
        std::string input;
        std::getline(std::cin, input);
        try {
            variables[var] = std::stod(input);
        } catch (...) {
            std::cerr << "Invalid input. Defaulting " << var << " to 0." << std::endl;
            variables[var] = 0;
        }
    }
}
void executeGOTO(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    int target;
    iss >> cmd >> target;
    if (programSource.count(target)) {
        currentLineNumber = target;
    } else {
        std::cerr << "ERROR: GOTO to undefined line " << target << std::endl;
        currentLineNumber = -1;
    }
}
void executeIF(const std::string&) { std::cout << "[IF stub]\n"; }
void executeFOR(const std::string& line) {
    if (loopStack.size() >= 15) {
        std::cerr << "ERROR: Maximum loop nesting (15) exceeded." << std::endl;
        currentLineNumber = -1;
        return;
    }

    std::istringstream iss(line);
    std::string cmd, var, eq, tokw;
    double start, final, step = 1;
    iss >> cmd >> var >> eq >> start >> tokw >> final;

    std::string remaining;
    std::getline(iss, remaining);
    size_t step_pos = remaining.find("STEP");
    if (step_pos != std::string::npos) {
        std::istringstream sstep(remaining.substr(step_pos + 4));
        sstep >> step;
    }

    variables[var] = start;

    LoopFrame frame;
    frame.var = var;
    frame.final = final;
    frame.step = step;
    frame.returnLine = currentLineNumber;
    loopStack.push_back(frame);
}

void executeDEF(const std::string&) { std::cout << "[DEF stub]\n"; }
void executeDIM(const std::string& line) {
    std::string rest = line.substr(3);
    std::stringstream ss(rest);
    std::string varname, dims;
    if (std::getline(ss, varname, '(')) {
        varname.erase(0, varname.find_first_not_of(" \t"));
        varname.erase(varname.find_last_not_of(" \t") + 1);
        if (std::getline(ss, dims, ')')) {
            std::stringstream dimstream(dims);
            std::string token;
            std::vector<int> shape;
            int total = 1;
            while (std::getline(dimstream, token, ',')) {
                token.erase(0, token.find_first_not_of(" \t"));
                token.erase(token.find_last_not_of(" \t") + 1);
                try {
                    int dim = std::stoi(token);
                    if (dim <= 0) throw std::runtime_error("Zero or negative dimension");
                    shape.push_back(dim);
                    total *= dim;
                } catch (...) {
                    std::cerr << "ERROR: Invalid dimension value: " << token << std::endl;
                    return;
                }
            }
            if (shape.size() > 15) {
                std::cerr << "ERROR: Too many dimensions (max 15)." << std::endl;
                return;
            }

            ArrayInfo arr;
            arr.shape = shape;
            if (total < 10000) {
                arr.data.resize(total, 0.0);
                std::cout << "Allocated dense array " << varname << " with " << total << " elements." << std::endl;
            } else {
                std::cout << "Using sparse storage for array " << varname << " with " << total << " elements." << std::endl;
            }

            arrays[varname] = arr;
        }
    }
}
void executeREM(const std::string&) { //std::cout << "[REM stub]\n"; }
void executeSTOP(const std::string&) {
    std::exit(0);
}
void executeGOSUB(const std::string& line) {
    if (gosubStack.size() >= 15) {
        std::cerr << "ERROR: GOSUB stack overflow (max 15 levels)." << std::endl;
        currentLineNumber = -1;
        return;
    }
    std::istringstream iss(line);
    std::string cmd;
    int target;
    iss >> cmd >> target;
    if (programSource.count(target)) {
        gosubStack.push(currentLineNumber);
        currentLineNumber = target;
    } else {
        std::cerr << "ERROR: GOSUB to undefined line " << target << std::endl;
        currentLineNumber = -1;
    }
}
void executeRETURN(const std::string&) {
    if (gosubStack.empty()) {
        std::cerr << "ERROR: RETURN without GOSUB" << std::endl;
        currentLineNumber = -1;
    } else {
        currentLineNumber = gosubStack.top();
        gosubStack.pop();
    }
}
void executeON(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, controlExpr, action, remainder;
    iss >> cmd >> controlExpr >> action;
    std::getline(iss, remainder);
    remainder.erase(0, remainder.find_first_not_of(" 	"));

    int index = static_cast<int>(evaluateExpression(controlExpr));
    std::stringstream targets(remainder);
    std::string token;
    std::vector<int> lines;
    while (std::getline(targets, token, ',')) {
        token.erase(0, token.find_first_not_of(" 	"));
        token.erase(token.find_last_not_of(" 	") + 1);
        try {
            lines.push_back(std::stoi(token));
        } catch (...) {
            std::cerr << "ERROR: Invalid line number in ON " << action << std::endl;
            return;
        }
    }

    if (index < 1 || index > static_cast<int>(lines.size())) {
        std::cerr << "ERROR: ON " << action << " index out of bounds: " << index << std::endl;
        return;
    }

    int targetLine = lines[index - 1];
    if (!programSource.count(targetLine)) {
        std::cerr << "ERROR: ON " << action << " target line does not exist: " << targetLine << std::endl;
        currentLineNumber = -1;
        return;
    }

    if (action == "GOTO") {
        currentLineNumber = targetLine;
    } else if (action == "GOSUB") {
        if (gosubStack.size() >= 15) {
            std::cerr << "ERROR: GOSUB stack overflow in ON GOSUB" << std::endl;
            currentLineNumber = -1;
            return;
        }
        gosubStack.push(currentLineNumber);
        currentLineNumber = targetLine;
    } else {
        std::cerr << "ERROR: Unknown ON action: " << action << std::endl;
    }
}

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
