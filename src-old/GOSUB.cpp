
void executeGOSUB(const std::string &line) {
  if (gosubStack.size() >= 15) {
    std::cerr << "ERROR: GOSUB stack overflow (max 15 levels)." << std::endl;
    currentLineNumber = -1;
    return;
  }
  std::istringstream iss(line);
  std::string cmd;
  int target;
  iss >> cmd >> target;
  if (programSource.count(target)) {
    gosubStack.push(currentLineNumber);
    currentLineNumber = target;
  } else {
    std::cerr << "ERROR: GOSUB to undefined line " << target << std::endl;
    currentLineNumber = -1;
  }
}

void executeRETURN(const std::string &) {
  if (gosubStack.empty()) {
    std::cerr << "ERROR: RETURN without GOSUB" << std::endl;
    currentLineNumber = -1;
  } else {
    currentLineNumber = gosubStack.top();
    gosubStack.pop();
  }
}
