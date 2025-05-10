#include "renumber.h"

void handleRENUMBER(int newStart, int delta, int oldStart) {
    if (program.programSource.empty()) {
        std::cerr << "ERROR: No program loaded.\n";
        return;
    }

    std::map<int, std::string> newSource;
    std::map<int, int> lineMapping;
    int nextLine = newStart;

    // Build mapping of old line → new line
    for (std::map<int,std::string>::const_iterator it = program.programSource.begin();
         it != program.programSource.end(); ++it) {
        int lineNum = it->first;
        if (lineNum >= oldStart) {
            lineMapping[lineNum] = nextLine;
            nextLine += delta;
        } else {
            lineMapping[lineNum] = lineNum;
        }
    }

    // Apply mapping and update references
    for (std::map<int,std::string>::const_iterator it = program.programSource.begin();
         it != program.programSource.end(); ++it) {
        int oldLine = it->first;
        std::string code = it->second;
        int newLine = lineMapping[oldLine];
        std::string updatedCode = code;

        // Regex to match line number references
        std::regex re(R"(\b(?:GOTO|GOSUB|THEN|PRINT\s+USING)\s+(\d+)|ON\s+[^,]+?\s+GOTO\s+((?:\d+\s*,\s*)*\d+)|ON\s+[^,]+?\s+GOSUB\s+((?:\d+\s*,\s*)*\d+))", std::regex::icase);
        updatedCode = std::regex_replace(updatedCode, re,
            std::function<std::string(const std::smatch&)>(
                [&lineMapping](const std::smatch& m) -> std::string {
                    // Single-line refs
                    if (m[1].matched) {
                        int oldRef = std::atoi(m[1].str().c_str());
                        if (lineMapping.count(oldRef)) {
                            return m.str().substr(0, m.position(1) - m.position(0))
                                 + std::to_string(lineMapping[oldRef]);
                        }
                    }
                    // ON GOTO / ON GOSUB lists
                    if (m[2].matched || m[3].matched) {
                        std::string lineList = m[2].matched ? m[2].str() : m[3].str();
                        std::stringstream ss(lineList);
                        std::string part, replaced;
                        while (std::getline(ss, part, ',')) {
                            int oldRef = std::atoi(part.c_str());
                            if (!replaced.empty()) replaced += ",";
                            replaced += lineMapping.count(oldRef)
                                      ? std::to_string(lineMapping[oldRef])
                                      : part;
                        }
                        std::string full = m.str();
                        size_t keywordEnd = full.find_first_of("0123456789");
                        if (keywordEnd == std::string::npos) keywordEnd = 0;
                        return full.substr(0, keywordEnd) + replaced;
                    }
                    return m.str();
                }
            )
        );

        newSource[newLine] = updatedCode;
    }

    program.programSource = std::move(newSource);
    std::cout << "RENUMBER complete.\n";
}
