#include "fileio.h"
#include <filesystem>
#include "interpreter.h"
#include "syntax.h"
#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "renumber.h"

#include "program_structure.h"

extern void handleRENUMBER(int newStart, int delta, int oldStart);
extern void executeOPEN(const std::string &line);

extern PROGRAM_STRUCTURE program;
//PROGRAM_STRUCTURE program;
void list(int start = 0, int end = INT_MAX);

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
      program.filename = filename;
    load(program);
    } 
    else if (command == "RENUMBER") {
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
        for (std::map<int, std::string>::const_iterator it = program.programSource.begin();
     it != program.programSource.end(); ++it) {
    int linenum = it->first;
    const std::string &content = it->second;
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
        program.filename = filename;
    load(program);
      }
      try {
        runInterpreter(program);
      } catch (const std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
      }
      } else if (command == "SYNTAX") {
        checkSyntax(program.programSource);
      }
      else {
        std::cout << "Unrecognized command: " << command << std::endl;
      }
    }
  }


// List lines between start and end
void list(int start, int end) {
    for (std::map<int, std::string>::const_iterator it = program.programSource.begin();
         it != program.programSource.end(); ++it) {
        int linenum = it->first;
        if (linenum >= start && linenum <= end) {
            std::cout << linenum << " " << it->second << std::endl;
        }
    }
}

  int main(int argc, char *argv[]) {
    if (argc > 1) {
      program.filename = argv[1];
    load(program);
    }
    interactiveLoop();
    return 0;
  }
