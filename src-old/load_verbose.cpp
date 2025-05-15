
void load(const std::string &filename) {
  std::ifstream infile(filename);
  if (!infile) {
    std::cerr << "ERROR: Cannot open file: " << filename << std::endl;
    return;
  }

  program.programSource.clear();
  program.filename = filename;
  program.filepath = std::filesystem::absolute(filename).string();

  std::string line;
  int count = 0;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    int linenum;
    iss >> linenum;
    std::string remainder;
    std::getline(iss, remainder);
    remainder.erase(0, remainder.find_first_not_of(" 	"));
    if (!remainder.empty()) {
      program.programSource[linenum] = remainder;
      ++count;
      if (count % 100 == 0) {
        std::cout << "Loaded " << count
                  << " lines so far...
                     ";
      }
    }
  }

  program.filesize_lines = program.programSource.size();
  infile.seekg(0, std::ios::end);
  program.filesize_bytes = infile.tellg();

  std::cout << "Loaded " << program.filesize_lines << " lines from " << filename
            << std::endl;
}
