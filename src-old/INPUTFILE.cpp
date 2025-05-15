
void executeINPUTFILE(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, hashChannel;
  iss >> cmd >> hashChannel;

  if (hashChannel.front() == '#') {
    hashChannel = hashChannel.substr(1);
  }

  int channel = std::stoi(hashChannel);
  auto it = openFiles.find(channel);
  if (it == openFiles.end() || !it->second.isFileOpen) {
    std::cerr << "ERROR: INPUT# attempted on unopened or closed channel #"
              << channel << std::endl;
    return;
  }

  std::string rest;
  std::getline(iss, rest);
  size_t comma = rest.find(',');
  if (comma != std::string::npos) {
    rest = rest.substr(comma + 1);
  }

  std::stringstream varList(rest);
  std::string varname;
  while (std::getline(varList, varname, ',')) {
    varname.erase(0, varname.find_first_not_of(" \t"));
    varname.erase(varname.find_last_not_of(" \t") + 1);

    std::string val;
    if (!(it->second.stream >> val)) {
      std::cerr << "ERROR: Failed to read value from channel #" << channel
                << std::endl;
      return;
    }

    VarInfo v;
    if (!varname.empty() && varname.back() == '$') {
      v.vT = VT_STRING;
      v.s = val;
    } else {
      try {
        v.d = std::stod(val);
        v.vT = VT_DOUBLE;
      } catch (...) {
        std::cerr << "ERROR: INPUT# value '" << val
                  << "' is not a number for variable '" << varname << "'"
                  << std::endl;
        return;
      }
    }

    variables[varname] = v;
  }
}
