
// ========================= Statement Handlers =========================

void executeFOR(const std::string &line) {
  if (loopStack.size() >= 15) {
    std::cerr << "ERROR: Maximum loop nesting (15) exceeded." << std::endl;
    currentLineNumber = -1;
    return;
  }

  std::istringstream iss(line);
  std::string cmd, var, eq, tokw;
  double start, final, step = 1;
  iss >> cmd >> var >> eq >> start >> tokw >> final;

  std::string remaining;
  std::getline(iss, remaining);
  size_t step_pos = remaining.find("STEP");
  if (step_pos != std::string::npos) {
    std::istringstream sstep(remaining.substr(step_pos + 4));
    sstep >> step;
  }

  variables[var] = start;

  LoopFrame frame;
  frame.var = var;
  frame.final = final;
  frame.step = step;
  frame.returnLine = currentLineNumber;
  loopStack.push_back(frame);
}
