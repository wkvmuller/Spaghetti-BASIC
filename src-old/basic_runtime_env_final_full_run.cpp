#include "interpreter.h"
#include "syntax.h"
#include <algorithm>
#include <climits>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "program_structure.h"

extern PROGRAM_STRUCTURE program;

void handleRENUMBER(int newStart, int delta, int oldStart) {
  if (program.programSource.empty()) {
    std::cerr << "ERROR: No program loaded.\n";
    return;
  }

  std::map<int, std::string> newSource;
  std::map<int, int> lineMapping;
  int nextLine = newStart;

  for (const auto &[line, code] : program.programSource) {
    if (line >= oldStart) {
      lineMapping[line] = nextLine;
      nextLine += delta;
    } else {
      lineMapping[line] = line;
    }
  }

  for (const auto &[oldLine, code] : program.programSource) {
    int newLine = lineMapping[oldLine];
    std::string updatedCode = code;

    std::regex re(
        R"(\b(?:GOTO|GOSUB|THEN|PRINT\s+USING)\s+(\d+)|ON\s+[^,]+?\s+GOTO\s+((?:\d+\s*,\s*)*\d+)|ON\s+[^,]+?\s+GOSUB\s+((?:\d+\s*,\s*)*\d+))",
        std::regex::icase);
    updatedCode =
        std::regex_replace(updatedCode, re, [&](const std::smatch &m) {
          if (m[1].matched) {
            int oldRef = std::stoi(m[1].str());
            if (lineMapping.count(oldRef)) {
              return m.str().substr(0, m.position(1) - m.position(0)) +
                     std::to_string(lineMapping[oldRef]);
            }
          } else if (m[2].matched || m[3].matched) {
            std::string lineList = m[2].matched ? m[2].str() : m[3].str();
            std::stringstream ss(lineList);
            std::string part, replaced;
            while (std::getline(ss, part, ',')) {
              int oldRef = std::stoi(part);
              if (!replaced.empty())
                replaced += ",";
              replaced += lineMapping.count(oldRef)
                              ? std::to_string(lineMapping[oldRef])
                              : part;
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
}
}

void handleRENUMBER(int newStart, int delta, int oldStart) {
  if (program.programSource.empty()) {
    std::cerr << "ERROR: No program loaded.\n";
    return;
  }

  std::map<int, std::string> newSource;
  std::map<int, int> lineMapping;
  int nextLine = newStart;

  for (const auto &[line, code] : program.programSource) {
    if (line >= oldStart) {
      lineMapping[line] = nextLine;
      nextLine += delta;
    } else {
      lineMapping[line] = line;
    }
  }

  for (const auto &[oldLine, code] : program.programSource) {
    int newLine = lineMapping[oldLine];
    std::string updatedCode = code;

    std::regex re(R"(\b(?:GOTO|GOSUB|THEN|PRINT\s+USING)\s+(\d+))",
                  std::regex::icase);
    updatedCode =
        std::regex_replace(updatedCode, re, [&](const std::smatch &m) {
          int oldRef = std::stoi(m[1].str());
          if (lineMapping.count(oldRef)) {
            return m.str().substr(0, m.position(1) - m.position(0)) +
                   std::to_string(lineMapping[oldRef]);
          }
          return m.str();
        });

    newSource[newLine] = updatedCode;
  }

  program.programSource = std::move(newSource);
  std::cout << "RENUMBER complete.\n";
}

void interactiveLoop() {
  std::string input;
  while (true) {
    std::cout << "READY. ";
    if (!std::getline(std::cin, input))
      break;

    std::istringstream iss(input);
    std::string command;
    iss >> command;
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);

    if (command == "EXIT" || command == "BYE") {
      break;
    } else if (command == "LOAD") {
      std::string filename;
      iss >> filename;
      load(filename);
    } else if (command == "RENUMBER") {
      int newStart = 10, delta = 10, oldStart = 0;
      char comma;
      if (iss >> newStart) {
        if (iss >> comma && comma == ',') {
          if (iss >> delta) {
            if (iss >> comma && comma == ',') {
              iss >> oldStart;
            }
          }
        }
      }
      handleRENUMBER(newStart, delta, oldStart);
    }

    else if (command == "SAVE") {
      std::string filename;
      iss >> filename;
      std::ofstream outfile(filename);
      if (!outfile) {
        std::cerr << "ERROR: Cannot open file for writing: " << filename
                  << std::endl;
      } else {
        for (const auto &[linenum, content] : program.programSource) {
          outfile << linenum << " " << content << std::endl;
        }
        std::cout << "Saved " << program.programSource.size() << " lines to "
                  << filename << std::endl;
      }
    } else if (command == "NEW") {
      program.programSource.clear();
      std::cout << "Memory cleared." << std::endl;
    } else if (command == "LIST") {
      int start = 0, end = INT_MAX;
      char comma;
      if (iss >> start) {
        if (iss >> comma && comma == ',') {
          iss >> end;
        } else {
          end = INT_MAX;
        }
      }
      list(start, end);
    } else if (command == "RUN") {
      std::string filename;
      if (iss >> filename) {
        program.programSource.clear();
        load(filename);
      }
      try {
        runInterpreter(program);
      } catch (const std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
      }
    } else if (command == "SYNTAX") {
      checkSyntax(program.programSource);
    } else {
      std::cout << "Unrecognized command: " << command << std::endl;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    load(argv[1]);
  }
  interactiveLoop();
  return 0;
}
