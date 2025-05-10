
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>
#include "program_structure.h"

// Load program from program.filename
void load(PROGRAM_STRUCTURE &program) {
    const std::string &filename = program.filename;
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "ERROR: Cannot open file: " << filename << std::endl;
        return;
    }

    program.programSource.clear();
    program.filepath = std::filesystem::absolute(filename).string();

    std::string line;
    int count = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int linenum;
        iss >> linenum;
        std::string remainder;
        std::getline(iss, remainder);
        remainder.erase(0, remainder.find_first_not_of(" 	"));
        if (!remainder.empty()) {
            program.programSource[linenum] = remainder;
            ++count;
            if (count % 100 == 0) {
                std::cout << "Loaded " << count << " lines so far...
";
            }
        }
    }

    program.filesize_lines = program.programSource.size();
    infile.seekg(0, std::ios::end);
    program.filesize_bytes = infile.tellg();

    std::cout << "Loaded " << program.filesize_lines << " lines from " << filename << std::endl;
}

// Save program to program.filename
void save(PROGRAM_STRUCTURE &program) {
    const std::string &filename = program.filename;
    if (filename.empty()) {
        std::cerr << "ERROR: No filename specified in PROGRAM_STRUCTURE." << std::endl;
        return;
    }

    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "ERROR: Cannot open file for writing: " << filename << std::endl;
        return;
    }

    int count = 0;
    for (const auto& entry : program.programSource) {
    auto linenum = entry.first;
    auto content = entry.second; {
        outfile << linenum << " " << content << "\n";
        ++count;
        if (count % 100 == 0) {
            std::cout << "Wrote " << count << " lines so far...
";
        }
    }

    program.filepath = std::filesystem::absolute(filename).string();
    program.filesize_lines = program.programSource.size();

    outfile.flush();
    std::ifstream checksize(filename, std::ios::binary | std::ios::ate);
    program.filesize_bytes = checksize ? checksize.tellg() : 0;

    std::cout << "Saved " << program.filesize_lines << " lines to " << filename << std::endl;
}
