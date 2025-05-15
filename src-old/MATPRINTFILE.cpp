
void executeMATPRINTFILE(const std::string &line) {
  // Parse command: MAT PRINT#<filenum>, A, B$, C, ...
  std::istringstream iss(line);
  std::string cmd, hash;
  int filenum;
  iss >> cmd >> hash >> filenum;

  // Collect array names
  std::vector<std::string> arraysToPrint;
  std::string name;
  while (std::getline(iss, name, ',')) {
    name.erase(0, name.find_first_not_of(" \t"));
    name.erase(name.find_last_not_of(" \t") + 1);
    if (!name.empty())
      arraysToPrint.push_back(name);
  }

  // Validate file
  if (!fileHandles.count(filenum) || !fileHandles[filenum].isFileOpen) {
    std::cerr << "ERROR: File #" << filenum << " not open." << std::endl;
    return;
  }
  std::ostream &out = *fileHandles[filenum].stream;

  // For each array name, print all elements row by row
  for (const auto &arrName : arraysToPrint) {
    if (!arrays.count(arrName)) {
      out << "[ERR: " << arrName << " undefined]\n";
      continue;
    }
    const ArrayInfo &mat = arrays[arrName];
    // Only support 2D for printing
    if (mat.shape.size() != 2) {
      out << "[ERR: " << arrName << " not 2D]\n";
      continue;
    }
    int rows = mat.shape[0], cols = mat.shape[1];
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        std::vector<int> idx = {r, c};
        ArgsInfo val = getSparseValue(arrName, idx);
        if (val.isstring)
          out << val.s;
        else
          out << val.d;
        if (c < cols - 1)
          out << " ";
      }
      out << "\n";
    }
  }
}
