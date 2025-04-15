#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <climits>
#include <algorithm>

std::vector<std::string> memoryList;

void load(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "ERROR: Cannot open file: " << filename << std::endl;
        return;
    }
    memoryList.clear();
    std::string line;
    while (std::getline(infile, line)) {
        line.erase(0, line.find_first_not_of(" 	
"));
        line.erase(line.find_last_not_of(" 	
") + 1);
        if (!line.empty()) {
            memoryList.push_back(line);
        }
    }
    std::cout << "Loaded " << memoryList.size() << " lines from " << filename << std::endl;
}

int getLineNumber(const std::string& line) {
    std::istringstream iss(line);
    int number = 0;
    iss >> number;
    return number;
}

void list(int start = 0, int end = INT_MAX) {
    for (const auto& line : memoryList) {
        int linenum = getLineNumber(line);
        if (linenum >= start && linenum <= end) {
            std::cout << line << std::endl;
        }
    }
}

void interactiveLoop() {
    std::string input;
    while (true) {
        std::cout << "READY. ";
        if (!std::getline(std::cin, input)) break;

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
                std::cerr << "ERROR: Cannot open file for writing: " << filename << std::endl;
            } else {
                for (const auto& line : memoryList) {
                    outfile << line << std::endl;
                }
                std::cout << "Saved " << memoryList.size() << " lines to " << filename << std::endl;
            }
        } else if (command == "NEW") {
            memoryList.clear();
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
        } else {
            std::cout << "Unrecognized command: " << command << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        load(argv[1]);
    }
    interactiveLoop();
    return 0;
}
