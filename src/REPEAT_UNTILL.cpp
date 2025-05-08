
void executeREPEAT(const std::string&) {
    if (loopStack.size() >= 15) {
        std::cerr << "ERROR: REPEAT nesting exceeded limit (15)." << std::endl;
        currentLineNumber = -1;
        return;
    }

    loopStack.push_back({"REPEAT", "", currentLineNumber});
}

void executeUNTIL(const std::string& line) {
    if (loopStack.empty() || loopStack.back().type != "REPEAT") {
        std::cerr << "ERROR: UNTIL without matching REPEAT" << std::endl;
        currentLineNumber = -1;
        return;
    }

    std::string cond = line.substr(5);
    cond.erase(0, cond.find_first_not_of(" \t"));

    if (evaluateExpression(cond) == 0.0) {
        currentLineNumber = loopStack.back().returnLine;
    } else {
        loopStack.pop_back();
    }
}
