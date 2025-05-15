
void executePRINTFILE(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, hashChannel;
  iss >> cmd >> hashChannel;

  if (hashChannel.front() == '#') {
    hashChannel = hashChannel.substr(1);
  }

  int channel = std::stoi(hashChannel);
  auto it = openFiles.find(channel);
  if (it == openFiles.end() || !it->second.isFileOpen) {
    std::cerr << "ERROR: PRINT# attempted on unopened or closed channel #"
              << channel << std::endl;
    return;
  }

  std::string rest;
  std::getline(iss, rest);
  size_t comma = rest.find(',');
  if (comma != std::string::npos) {
    rest = rest.substr(comma + 1);
  }

  std::stringstream ss(rest);
  std::string token;
  bool first = true;

  while (std::getline(ss, token, ',')) {
    token.erase(0, token.find_first_not_of(" \t"));
    token.erase(token.find_last_not_of(" \t") + 1);

    auto v = variables.find(token);
    if (v != variables.end()) {
      if (!first)
        it->second.stream << " ";
      first = false;

      if (v->second.vT == VT_STRING || v->second.vT == VT_TEXT) {
        it->second.stream << v->second.s;
      } else if (v->second.vT == VT_INT) {
        it->second.stream << v->second.ll;
      } else {
        it->second.stream << v->second.d;
      }
    } else {
      it->second.stream << token;
    }
  }

  it->second.stream << std::endl;
}
