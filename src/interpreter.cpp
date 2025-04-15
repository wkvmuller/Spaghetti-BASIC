#include <iostream>
#include <map>
#include <string>

void runInterpreter(const std::map<int, std::string>& programSource) {
    std::cout << "Interpreter called with " << programSource.size() << " lines of code." << std::endl;
    for (const auto& [linenum, content] : programSource) {
        std::cout << linenum << " " << content << std::endl;
    }
    // TODO: Implement actual BASIC interpreter logic here
}
