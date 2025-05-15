void executeMAT(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, sub;
  iss >> cmd >> sub; // cmd == "MAT", sub == next token

  if (sub == "READ") {
    // MAT READ X
    executeMATREAD(line);
  } else if (sub == "PRINT") {
    // MAT PRINT  …  could be console or file
    // Peek past “PRINT”
    iss >> std::ws;
    if (iss.peek() == '#') {
      // MAT PRINT #n, A, B…
      executeMATPRINTFILE(line);
    } else {
      // MAT PRINT A, B…
      executeMATPRINT(line);
    }
  } else {
    // MAT <target> = <expr>
    std::string target = sub, eq;
    iss >> eq; // consume “=”
    std::string expr;
    std::getline(iss, expr);
    expr.erase(0, expr.find_first_not_of(" \t"));
    evaluateMATExpression(target, expr);
  }
}
