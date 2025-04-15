#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <iomanip>
#include <algorithm>

int getLineNumber(const std::string& line) {
    std::istringstream iss(line);
    int num;
    iss >> num;
    return num;
}

std::string replaceLineNumber(const std::string& line, int newLineNumber) {
    std::ostringstream oss;
    oss << newLineNumber;
    std::string remainder = line.substr(line.find_first_of(" 	"));
    return oss.str() + remainder;
}

void renumberSource(std::vector<std::string>& lines, int newStart = 10, int delta = 10, int oldStart = 0) {
    std::map<int, std::string> renumbered;
    std::map<int, int> lineMapping;

    // First pass: assign new line numbers
    int newLine = newStart;
    for (const auto& line : lines) {
        int oldLine = getLineNumber(line);
        if (oldLine >= oldStart) {
            lineMapping[oldLine] = newLine;
            newLine += delta;
        } else {
            lineMapping[oldLine] = oldLine;
        }
    }

    // Second pass: apply new line numbers
    std::vector<std::string> newLines;
    for (const auto& line : lines) {
        int oldLine = getLineNumber(line);
        int newLineNum = lineMapping[oldLine];

        std::string updated = line;
        std::ostringstream oss;
        oss << newLineNum;
        size_t pos = updated.find_first_of(" 	");
        if (pos != std::string::npos)
            updated = oss.str() + updated.substr(pos);

        // Regex update for line number references
        for (const auto& [oldNum, newNum] : lineMapping) {
            std::string oldStr = std::to_string(oldNum);
            std::string newStr = std::to_string(newNum);
            std::regex goto_rgx("(GOTO|THEN|GOSUB|PRINT USING)\s+" + oldStr + "(?!\d)");

            std::regex print_file_using_rgx("PRINT\s+#\d+\s+USING\s+" + oldStr + "(?!\d)");
            updated = std::regex_replace(updated, print_file_using_rgx, "PRINT #$1 USING " + newStr);
            std::regex on_rgx("ON\s+[^\n]*\s+(GOTO|GOSUB)\s+([^\n]*)");

            updated = std::regex_replace(updated, goto_rgx, "$1 " + newStr);

            // Handle ON GOTO/GOSUB with line lists
            std::smatch match;
            if (std::regex_search(updated, match, on_rgx)) {
                std::string prefix = match.prefix();
                std::string suffix = match.suffix();
                std::string on_clause = match.str(2);
                std::stringstream ss(on_clause);
                std::string num;
                std::vector<std::string> parts;
                while (std::getline(ss, num, ',')) {
                    int ref = std::stoi(num);
                    auto it = lineMapping.find(ref);
                    if (it != lineMapping.end()) {
                        parts.push_back(std::to_string(it->second));
                    } else {
                        parts.push_back(num);
                    }
                }
                std::string new_clause = match.str(1) + " " + parts[0];
                for (size_t i = 1; i < parts.size(); ++i) {
                    new_clause += "," + parts[i];
                }
                updated = match.prefix().str() + "ON " + new_clause + match.suffix().str();
            }
        }

        newLines.push_back(updated);
    }

    lines = newLines;
}
