#include "renumber.h"
#include <regex>
#include <sstream>
#include <iterator>
#include <cstdlib>  // for atoi

void handleRENUMBER(int newStart, int delta, int oldStart) {
    if (program.programSource.empty()) {
        std::cerr << "ERROR: No program loaded.\n";
        return;
    }

    std::map<int, std::string> newSource;
    std::map<int, int> lineMapping;
    int nextLine = newStart;

    // Build mapping of old line -> new line
    for (std::map<int, std::string>::const_iterator it = program.programSource.begin();
         it != program.programSource.end(); ++it) {
        int lineNum = it->first;
        if (lineNum >= oldStart) {
            lineMapping[lineNum] = nextLine;
            nextLine += delta;
        } else {
            lineMapping[lineNum] = lineNum;
        }
    }

    // Regex to match line number references
    std::regex re(
        R"(\b(?:GOTO|GOSUB|THEN|PRINT\s+USING)\s+(\d+)|"
        R"(ON\s+[^,]+?\s+GOTO\s+((?:\d+\s*,\s*)*\d+))|"
        R"(ON\s+[^,]+?\s+GOSUB\s+((?:\d+\s*,\s*)*\d+)))",
        std::regex::icase
    );

    // Process each line
    for (std::map<int, std::string>::const_iterator it = program.programSource.begin();
         it != program.programSource.end(); ++it) {
        int oldLine = it->first;
        std::string updatedCode = it->second;
        int newLine = lineMapping[oldLine];

        // Manual regex_replace with iteration
        std::string result;
        std::string subject = updatedCode;
        std::size_t lastPos = 0;
        std::sregex_iterator rit(subject.begin(), subject.end(), re);
        std::sregex_iterator rend;
        for (; rit != rend; ++rit) {
            std::smatch m = *rit;
            // Append text before match
            result.append(subject.substr(lastPos, m.position() - lastPos));

            // Determine replacement
            std::string rep;
            if (m[1].matched) {
                int ref = std::atoi(m[1].str().c_str());
                if (lineMapping.count(ref)) {
                    rep = m.str().substr(0, m.position(1) - m.position(0))
                        + std::to_string(lineMapping[ref]);
                } else {
                    rep = m.str();
                }
            } else if (m[2].matched || m[3].matched) {
                std::string list = (m[2].matched ? m[2].str() : m[3].str());
                std::stringstream ss(list);
                std::string part, replaced;
                while (std::getline(ss, part, ',')) {
                    int ref = std::atoi(part.c_str());
                    if (!replaced.empty()) replaced += ',';
                    replaced += (lineMapping.count(ref)
                                 ? std::to_string(lineMapping[ref])
                                 : part);
                }
                std::string full = m.str();
                std::size_t keywordEnd = full.find_first_of("0123456789");
                if (keywordEnd == std::string::npos) keywordEnd = 0;
                rep = full.substr(0, keywordEnd) + replaced;
            } else {
                rep = m.str();
            }

            result += rep;
            lastPos = m.position() + m.length();
        }
        // Append remainder
        result += subject.substr(lastPos);

        newSource[newLine] = result;
    }

    program.programSource = newSource;
    std::cout << "RENUMBER complete.\n";
}
