
// Restore DATA pointer
void executeRESTORE(const std::string &) { dataPointer = 0; }

// Parse DATA statements into the buffer
void executeDATA(const std::string &line) {
  // Remove leading "DATA"
  std::string rest = line.substr(4);
  // Split on commas
  std::stringstream ss(rest);
  std::string token;
  while (std::getline(ss, token, ',')) {
    // Trim whitespace
    token.erase(0, token.find_first_not_of(" \t"));
    token.erase(token.find_last_not_of(" \t") + 1);
    if (token.size() >= 2 && token.front() == '\"' && token.back() == '\"') {
      // String literal
      std::string content = token.substr(1, token.size() - 2);
      dataBuffer.push_back(makeArgsInfo(currentline, "", true, content, 0.0));
    } else {
      // Numeric value
      double val = 0.0;
      try {
        val = std::stod(token);
      } catch (...) {
        std::cerr << "ERROR: Invalid DATA value: " << token << " on line "
                  << currentline << std::endl;
      }
      dataBuffer.push_back(makeArgsInfo(currentline, "", false, "", val));
    }
  }
}
