#include "syntax.h"
#include <iostream>
#include <regex>
#include <map>
#include <string>
#include <set>
#include <sstream>

void checkSyntax(const std::map<int, std::string>& programSource) {
    std::set<int> referencedLines;
    std::set<int> definedLines;

    for (std::map<int, std::string>::const_iterator it = programSource.begin(); it != programSource.end(); ++it) {
        int lineNumber = it->first;
        std::string content = it->second;
        definedLines.insert(lineNumber);

    std::set<std::string> validMathFunctions = {
        "SIN", "COS", "TAN", "ATN", "ASN", "ACS",
        "COT", "SEC", "CSC", "LOG", "LOG10", "CLOG",
        "EXP", "RND", "INT", "DEG2RAD", "RAD2DEG",
        "ASCII", "VALUE", "POW", "ROUND", "FLOOR", "CEIL", "SQR", "TIME"
    };
    std::set<std::string> validStringFunctions = {
        "LEFT$", "RIGHT$", "MID$", "LEN$", "CHR$", "STRING$", "TIME$", "DATE$", "TEST$"
    };


        std::string upper = content;
        for (size_t i = 0; i < upper.length(); ++i) upper[i] = toupper(upper[i]);

        std::smatch match;
        std::regex goto_rgx(R"(GOTO\s+(\d+))");
        std::regex then_rgx(R"(THEN\s+(\d+))");
        std::regex gosub_rgx(R"(GOSUB\s+(\d+))");
        std::regex print_using_rgx(R"(PRINT\s+USING\s+(\d+))");
        std::regex on_goto_rgx(R"(ON\s+\w+\s+GOTO\s+([\d,]+))");
        std::regex on_gosub_rgx(R"(ON\s+\w+\s+GOSUB\s+([\d,]+))");

        auto extract_refs = [&](const std::regex& rgx) {
            std::smatch m;
            std::string copy = upper;
            while (std::regex_search(copy, m, rgx)) {
                std::string nums = m[1].str();
                std::stringstream ss(nums);
                std::string num;
                while (std::getline(ss, num, ',')) {
                    try {
                        referencedLines.insert(std::stoi(num));
                    } catch (...) {}
                }
                copy = m.suffix().str();
            }
        };

        extract_refs(goto_rgx);
        extract_refs(then_rgx);
        extract_refs(gosub_rgx);
        extract_refs(print_using_rgx);
        extract_refs(on_goto_rgx);
        extract_refs(on_gosub_rgx);

        std::regex func_rgx(R"((\b[A-Z]+\$?)\s*\()");
        std::smatch func_match;
        std::string check = upper;
        while (std::regex_search(check, func_match, func_rgx)) {
            std::string fname = func_match[1];
            if (validMathFunctions.find(fname) == validMathFunctions.end() &&
                validStringFunctions.find(fname) == validStringFunctions.end()) {
                std::cout << "SYNTAX ERROR: Unknown function '" << fname << "' in line " << lineNumber << ": " << content << std::endl;
                ok = false;
            }
            check = func_match.suffix().str();
        }

    }

    bool ok = true;
    for (int ref : referencedLines) {
        if (definedLines.find(ref) == definedLines.end()) {
            std::cout << "SYNTAX ERROR: Missing referenced line " << ref << std::endl;
            ok = false;
        }
    }

    if (ok) {
        std::cout << "SYNTAX CHECK COMPLETE. No errors found." << std::endl;
    }
}
