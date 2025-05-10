#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

void renumberSource(std::map<int, std::string> &program, int newStart = 10,
                    int delta = 10, int oldStart = 0) {
  std::map<int, std::string> newProgram;
  std::map<int, int> lineMapping;

  int newLine = newStart;
  for (const auto &[oldLine, content] : program) {
    if (oldLine >= oldStart) {
      lineMapping[oldLine] = newLine;
      newLine += delta;
    } else {
      lineMapping[oldLine] = oldLine;
    }
  }

  for (const auto &[oldLine, content] : program) {
    int newLineNum = lineMapping[oldLine];
    std::string updated = content;

    for (const auto &[oldNum, newNum] : lineMapping) {
      std::string oldStr = std::to_string(oldNum);
      std::string newStr = std::to_string(newNum);

      std::regex goto_rgx("(GOTO|THEN|GOSUB|PRINT USING)\s+" + oldStr +
                          "(?!\d)");
      std::regex print_file_using_rgx("PRINT\s+#\d+\s+USING\s+" + oldStr +
                                      "(?!\d)");
      updated = std::regex_replace(updated, goto_rgx, "$1 " + newStr);
      updated = std::regex_replace(updated, print_file_using_rgx,
                                   "PRINT #$1 USING " + newStr);

      std::regex on_rgx("ON\s+[^\n]*\s+(GOTO|GOSUB)\s+([^\n]*)");
      std::smatch match;
      if (std::regex_search(updated, match, on_rgx)) {
        std::string on_clause = match.str(2);
        std::stringstream ss(on_clause);
        std::string num;
        std::vector<std::string> parts;
        while (std::getline(ss, num, ',')) {
          try {
            int ref = std::stoi(num);
            parts.push_back(std::to_string(
                lineMapping.count(ref) ? lineMapping[ref] : ref));
          } catch (...) {
            parts.push_back(num);
          }
        }
        std::string new_clause = match.str(1) + " " + parts[0];
        for (size_t i = 1; i < parts.size(); ++i)
          new_clause += "," + parts[i];
        updated =
            match.prefix().str() + "ON " + new_clause + match.suffix().str();
      }
    }

    newProgram[newLineNum] = updated;
  }

  program = newProgram;
}void handleRENUMBER(int newStart, int delta, int oldStart) {
    if (program.programSource.empty()) {
        std::cerr << "ERROR: No program loaded.\n";
        return;
    }

    std::map<int, std::string> newSource;
    std::map<int, int> lineMapping;
    int nextLine = newStart;

    for (const auto& [line, code] : program.programSource) {
        if (line >= oldStart) {
            lineMapping[line] = nextLine;
            nextLine += delta;
        } else {
            lineMapping[line] = line;
        }
    }

    for (const auto& [oldLine, code] : program.programSource) {
        int newLine = lineMapping[oldLine];
        std::string updatedCode = code;

        std::regex re(R"(\b(?:GOTO|GOSUB|THEN|PRINT\s+USING)\s+(\d+)|ON\s+[^,]+?\s+GOTO\s+((?:\d+\s*,\s*)*\d+)|ON\s+[^,]+?\s+GOSUB\s+((?:\d+\s*,\s*)*\d+))", std::regex::icase);
        updatedCode = std::regex_replace(updatedCode, re, [&](const std::smatch& m) {
            if (m[1].matched) {
                int oldRef = std::stoi(m[1].str());
                if (lineMapping.count(oldRef)) {
                    return m.str().substr(0, m.position(1) - m.position(0)) + std::to_string(lineMapping[oldRef]);
                }
            } else if (m[2].matched || m[3].matched) {
                std::string lineList = m[2].matched ? m[2].str() : m[3].str();
                std::stringstream ss(lineList);
                std::string part, replaced;
                while (std::getline(ss, part, ',')) {
                    int oldRef = std::stoi(part);
                    if (!replaced.empty()) replaced += ",";
                    replaced += lineMapping.count(oldRef) ? std::to_string(lineMapping[oldRef]) : part;
                }
                size_t keywordEnd = m.str().find_first_of("0123456789");
                return m.str().substr(0, keywordEnd) + replaced;
            }
            return m.str();
        });

        newSource[newLine] = updatedCode;
    }

    program.programSource = std::move(newSource);
    std::cout << "RENUMBER complete.\n";
}include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

void renumberSource(std::map<int, std::string> &program, int newStart = 10,
                    int delta = 10, int oldStart = 0) {
  std::map<int, std::string> newProgram;
  std::map<int, int> lineMapping;

  int newLine = newStart;
  for (const auto &[oldLine, content] : program) {
    if (oldLine >= oldStart) {
      lineMapping[oldLine] = newLine;
      newLine += delta;
    } else {
      lineMapping[oldLine] = oldLine;
    }
  }

  for (const auto &[oldLine, content] : program) {
    int newLineNum = lineMapping[oldLine];
    std::string updated = content;

    for (const auto &[oldNum, newNum] : lineMapping) {
      std::string oldStr = std::to_string(oldNum);
      std::string newStr = std::to_string(newNum);

      std::regex goto_rgx("(GOTO|THEN|GOSUB|PRINT USING)\s+" + oldStr +
                          "(?!\d)");
      std::regex print_file_using_rgx("PRINT\s+#\d+\s+USING\s+" + oldStr +
                                      "(?!\d)");
      updated = std::regex_replace(updated, goto_rgx, "$1 " + newStr);
      updated = std::regex_replace(updated, print_file_using_rgx,
                                   "PRINT #$1 USING " + newStr);

      std::regex on_rgx("ON\s+[^\n]*\s+(GOTO|GOSUB)\s+([^\n]*)");
      std::smatch match;
      if (std::regex_search(updated, match, on_rgx)) {
        std::string on_clause = match.str(2);
        std::stringstream ss(on_clause);
        std::string num;
        std::vector<std::string> parts;
        while (std::getline(ss, num, ',')) {
          try {
            int ref = std::stoi(num);
            parts.push_back(std::to_string(
                lineMapping.count(ref) ? lineMapping[ref] : ref));
          } catch (...) {
            parts.push_back(num);
          }
        }
        std::string new_clause = match.str(1) + " " + parts[0];
        for (size_t i = 1; i < parts.size(); ++i)
          new_clause += "," + parts[i];
        updated =
            match.prefix().str() + "ON " + new_clause + match.suffix().str();
      }
    }

    newProgram[newLineNum] = updated;
  }

  program = newProgram;
}
