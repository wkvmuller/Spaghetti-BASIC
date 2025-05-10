#include "program_structure.h"

extern PROGRAM_STRUCTURE program;
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
}
