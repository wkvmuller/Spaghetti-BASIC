
// ========================= Statement Handlers =========================

void executeDIM(const std::string &line) {
  // Parse variable name and dimension list
  std::string rest = line.substr(3);
  std::stringstream ss(rest);
  std::string varname, dims;
  if (!std::getline(ss, varname, '('))
    return;
  varname.erase(0, varname.find_first_not_of(" \t"));
  varname.erase(varname.find_last_not_of(" \t") + 1);
  if (!std::getline(ss, dims, ')'))
    return;

  // Build shape vector and compute total elements
  std::stringstream dimstream(dims);
  std::string token;
  std::vector<int> shape;
  long long total = 1;
  while (std::getline(dimstream, token, ',')) {
    token.erase(0, token.find_first_not_of(" \t"));
    token.erase(token.find_last_not_of(" \t") + 1);
    int dim = 0;
    try {
      dim = std::stoi(token);
    } catch (...) {
      std::cerr << "ERROR: Invalid dimension value: " << token << std::endl;
      return;
    }
    if (dim <= 0) {
      std::cerr << "ERROR: Dimension must be positive: " << dim << std::endl;
      return;
    }
    shape.push_back(dim);
    total *= dim;
  }

  if (shape.size() > 11) {
    std::cerr << "ERROR: Too many dimensions (max 11)." << std::endl;
    return;
  }

  // Create and initialize ArrayInfo
  ArrayInfo arr;
  arr.shape = shape;

  bool isString = (!varname.empty() && varname.back() == '$');
  if (total < 10000) {
    if (isString) {
      arr.dataStr.assign(static_cast<size_t>(total), std::string());
    } else {
      arr.data.assign(static_cast<size_t>(total), 0.0);
    }
    arr.sparse.clear();
    arr.stringSparse.clear();
    std::cout << "DIM: allocated " << (isString ? "string" : "numeric")
              << " dense array " << varname << " with " << total << " elements."
              << std::endl;
  } else {
    if (isString) {
      arr.dataStr.clear();
    } else {
      arr.data.clear();
    }
    arr.sparse.clear();
    arr.stringSparse.clear();
    std::cout << "DIM: initialized " << (isString ? "string" : "numeric")
              << " sparse array " << varname << " capable of " << total
              << " elements." << std::endl;
  }

  arrays[varname] = std::move(arr);
}
