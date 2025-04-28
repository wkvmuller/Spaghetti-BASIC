
// ========================= Statement Handlers =========================

void executeGO(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd;
  int target;
  iss >> cmd >> target;

  if (programSource.count(target)) {
    currentLineNumber = target;
  } else {
    std::cerr << "ERROR: GO to undefined line " << target << std::endl;
    currentLineNumber = -1;
  }
}
