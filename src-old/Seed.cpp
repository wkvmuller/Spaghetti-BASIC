
void executeSEED(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd;
  int seed;
  iss >> cmd >> seed;

  srand(seed); // seed RNG
  std::cout << "RNG seeded with value: " << seed << std::endl;
}
