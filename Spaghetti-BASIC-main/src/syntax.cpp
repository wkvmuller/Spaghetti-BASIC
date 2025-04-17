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

    std::set<std::string> validMathFunctions = {
        "SIN", "COS", "TAN", "ATN", "ASN", "ACS",
        "COT", "SEC", "CSC", "LOG", "LOGX", "LOG10", "CLOG",
        "EXP", "RND", "INT", "DEG2RAD", "RAD2DEG",
        "ASCII", "VALUE", "POW", "ROUND", "FLOOR", "CEIL", "TIME", "SQR"
    };
    std::set<std::string> validStringFunctions = {
        "LEFT$", "RIGHT$", "MID$", "LEN$", "CHR$", "STRING$",
        "TIME$", "DATE$", "TEST$"
    };

    std::vector<std::string> controlStack;
    bool ok = true;

    for (std::map<int, std::string>::const_iterator it = programSource.begin(); it != programSource.end(); ++it) {
        int lineNumber = it->first;
        std::string line = it->second;
        definedLines.insert(lineNumber);

        std::regex dim_rgx(R"(DIM\s+\w+\s*\(([^\)]*)\))");
        std::smatch dim_match;
        if (std::regex_search(upper, dim_match, dim_rgx)) {
            std::string dims = dim_match[1];
            std::stringstream ss(dims);
            std::string d;
            int count = 0;
            while (std::getline(ss, d, ',')) ++count;
            if (count > 15) {
                std::cout << "SYNTAX ERROR: DIM exceeds 15 dimensions at line "
                          << lineNumber << ": " << line << std::endl;
                ok = false;
            }
        }


        std::string upper = line;
        for (size_t i = 0; i < upper.length(); ++i) upper[i] = toupper(upper[i]);

        std::regex ref_rgx(R"((GOTO|THEN|GOSUB|PRINT\s+USING|PRINT\s+#\d+\s+USING)\s+(\d+))");
        std::smatch ref_match;
        std::string check = upper;
        while (std::regex_search(check, ref_match, ref_rgx)) {
            try {
                referencedLines.insert(std::stoi(ref_match[2].str()));
            } catch (...) {}
            check = ref_match.suffix().str();
        }

        std::regex on_rgx(R"(ON\s+\w+\s+(GOTO|GOSUB)\s+([\d,]+))");
        std::smatch on_match;
        check = upper;
        while (std::regex_search(check, on_match, on_rgx)) {
            std::stringstream ss(on_match[2]);
            std::string num;
            while (std::getline(ss, num, ',')) {
                try {
                    referencedLines.insert(std::stoi(num));
                } catch (...) {}
            }
            check = on_match.suffix().str();
        }

        std::regex func_rgx(R"(([A-Z]+\$?)\s*\()");
        std::smatch func_match;
        check = upper;
        while (std::regex_search(check, func_match, func_rgx)) {
            std::string fname = func_match[1];
            if (validMathFunctions.find(fname) == validMathFunctions.end() &&
                validStringFunctions.find(fname) == validStringFunctions.end()) {
                std::cout << "SYNTAX ERROR: Unknown function '" << fname
                          << "' in line " << lineNumber << ": " << line << std::endl;
                ok = false;
            }
            check = func_match.suffix().str();
        }

        std::regex let_rgx(R"(^LET\s+\w+\s*=\s*.+)");
        std::regex if_rgx(R"(^IF\s+.+\s+THEN\s+.+)");
        std::regex input_rgx(R"(^INPUT(\s+".*"\s*;\s*)?\s*\w+(\s*,\s*\w+)*)");
        std::regex for_rgx(R"(^FOR\s+\w+\s*=\s*.+\s+TO\s+.+(\s+STEP\s+.+)?)");
        std::regex next_rgx(R"(^NEXT\s+\w+)");
        std::regex while_rgx(R"(^WHILE\s+.+)");
        std::regex wend_rgx(R"(^WEND)");
        std::regex repeat_rgx(R"(^REPEAT)");
        std::regex until_rgx(R"(^UNTIL\s+.+)");

        auto check_match = [&](const std::regex& rgx, const std::string& tag) -> bool {
            if (!std::regex_match(upper, rgx)) {
                std::cout << "SYNTAX ERROR: Invalid " << tag << " syntax at line "
                          << lineNumber << ": " << line << std::endl;
                return false;
            }
            return true;
        };

        if (upper.find("LET ") == 0) ok &= check_match(let_rgx, "LET");
        else if (upper.find("IF ") == 0) ok &= check_match(if_rgx, "IF");
        else if (upper.find("INPUT") == 0) ok &= check_match(input_rgx, "INPUT");
        else if (upper.find("FOR ") == 0) ok &= check_match(for_rgx, "FOR");
        else if (upper.find("NEXT") == 0) ok &= check_match(next_rgx, "NEXT");
        else if (upper.find("WHILE ") == 0) { ok &= check_match(while_rgx, "WHILE"); controlStack.push_back("WHILE");
            if (controlStack.size() > 15) {
                std::cout << "SYNTAX ERROR: Loop nesting exceeds 15 levels at line " << lineNumber << ": " << line << std::endl;
                ok = false;
            } }
        else if (upper == "WEND") {
            if (!controlStack.empty() && controlStack.back() == "WHILE") controlStack.pop_back();
            else {
                std::cout << "SYNTAX ERROR: WEND without matching WHILE at line "
                          << lineNumber << ": " << line << std::endl;
                ok = false;
            }
        } else if (upper == "REPEAT") {
            controlStack.push_back("REPEAT");
            if (controlStack.size() > 15) {
                std::cout << "SYNTAX ERROR: Loop nesting exceeds 15 levels at line " << lineNumber << ": " << line << std::endl;
                ok = false;
            }
        } else if (upper.find("UNTIL ") == 0) {
            if (!controlStack.empty() && controlStack.back() == "REPEAT") controlStack.pop_back();
            else {
                std::cout << "SYNTAX ERROR: UNTIL without matching REPEAT at line "
                          << lineNumber << ": " << line << std::endl;
                ok = false;
            }
        }
    }

    for (int ref : referencedLines) {
        if (definedLines.find(ref) == definedLines.end()) {
            std::cout << "SYNTAX ERROR: Missing referenced line " << ref << std::endl;
            ok = false;
        }
    }

    if (!controlStack.empty()) {
        for (std::set<std::string>::iterator it = controlStack.begin(); it != controlStack.end(); ++it) {
            std::cout << "SYNTAX ERROR: Missing closing for " << *it << " block." << std::endl;
        }
        ok = false;
    }

    if (ok) {
        std::cout << "SYNTAX CHECK COMPLETE. No errors found." << std::endl;
    }
}
