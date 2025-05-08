
void executeWHILE(const std::string& line) {
    if (loopStack.size() >= 15) {
        std::cerr << "ERROR: WHILE nesting exceeded limit (15)." << std::endl;
        currentLineNumber = -1;
        return;
    }

    std::string cond = line.substr(5);
    cond.erase(0, cond.find_first_not_of(" \t"));

    if (evaluateExpression(cond) == 0.0) {
        int depth = 1;
        auto it = programSource.upper_bound(currentLineNumber);
        while (it != programSource.end()) {
            std::string upper = it->second;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            if (upper.find("WHILE") == 0) depth++;
            else if (upper.find("WEND") == 0) {
                depth--;
                if (depth == 0) {
                    currentLineNumber = it->first;
                    return;
                }
            }
            ++it;
        }
        std::cerr << "ERROR: WHILE without matching WEND." << std::endl;
        currentLineNumber = -1;
        return;
    }

    loopStack.push_back({"WHILE", cond, currentLineNumber});
}

void executeWEND(const std::string&) {
    if (loopStack.empty() || loopStack.back().type != "WHILE") {
        std::cerr << "ERROR: WEND without matching WHILE" << std::endl;
        currentLineNumber = -1;
        return;
    }

    LoopFrame frame = loopStack.back();
    if (evaluateExpression(frame.condition) != 0.0) {
        currentLineNumber = frame.returnLine;
    } else {
        loopStack.pop_back();
    }
}
