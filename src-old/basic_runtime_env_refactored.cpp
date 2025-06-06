#include "interpreter.h"
#include "syntax.h"
#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "program_structure.h"

extern PROGRAM_STRUCTURE program;

void load(const std::string &filename) {
  std::ifstream infile(filename);
  if (!infile) {
    std::cerr << "ERROR: Cannot open file: " << filename << std::endl;
    return;
  }
  program.programSource.clear();
  std::string line;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    int linenum;
    iss >> linenum;
    std::string remainder;
    std::getline(iss, remainder);
    remainder.erase(0, remainder.find_first_not_of(" 	"));
    if (!remainder.empty()) {
      program.programSource[linenum] = remainder;
    }
  }
  std::cout << "Loaded " << program.programSource.size() << " lines from "
            << filename << std::endl;
}

void list(int start = 0, int end = INT_MAX) {
  for (const auto &entry : program.programSource) {
    int linenum = entry.first;
    const std::string &content = entry.second;
    if (linenum >= start && linenum <= end) {
      std::cout << linenum << " " << content << std::endl;
    }
  }
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
    } else if (command == "SAVE") {
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
        runInterpreter(program.programSource);
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
