
void executeON(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, exprToken, mode;
  iss >> cmd >> exprToken >> mode;

  std::string targetList;
  std::getline(iss, targetList);
  targetList.erase(0, targetList.find_first_not_of(" 	"));

  int index = static_cast<int>(evaluateExpression(exprToken), currentline);
  if (index < 1) {
    std::cerr << "ERROR: ON " << mode << " index must be ≥ 1: " << index
              << std::endl;
    return;
  }

  std::stringstream targets(targetList);
  std::string token;
  std::vector<int> lineNumbers;
  while (std::getline(targets, token, ',')) {
    token.erase(0, token.find_first_not_of(" 	"));
    token.erase(token.find_last_not_of(" 	") + 1);
    try {
      lineNumbers.push_back(std::stoi(token));
    } catch (...) {
      std::cerr << "ERROR: Invalid line number in ON " << mode << std::endl;
      return;
    }
  }

  if (index > static_cast<int>(lineNumbers.size())) {
    std::cerr << "ERROR: ON " << mode << " index out of bounds: " << index
              << std::endl;
    return;
  }

  int targetLine = lineNumbers[index - 1];
  if (!programSource.count(targetLine)) {
    std::cerr << "ERROR: ON " << mode << " line " << targetLine
              << " does not exist." << std::endl;
    currentLineNumber = -1;
    return;
  }

  if (mode == "GOTO") {
    currentLineNumber = targetLine;
  } else if (mode == "GOSUB") {
    if (gosubStack.size() >= 15) {
      std::cerr << "ERROR: GOSUB stack overflow in ON GOSUB" << std::endl;
      currentLineNumber = -1;
      return;
    }
    gosubStack.push(currentLineNumber);
    currentLineNumber = targetLine;
  } else {
    std::cerr << "ERROR: Unsupported ON mode: " << mode << std::endl;
  }
}
