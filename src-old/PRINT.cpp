
// ========================= Statement Handlers =========================

void executePRINT(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd;
  iss >> cmd;
  std::string items;
  std::getline(iss, items);
  std::stringstream ss(items);
  std::string token;
  while (std::getline(ss, token, ',')) {
    token.erase(0, token.find_first_not_of(" 	"));
    token.erase(token.find_last_not_of(" 	") + 1);
    std::string name;
    std::vector<int> indices;
    if (parseIndexedArray(token, name, indices)) {
      ArgsInfo val = getSparseValue(name, indices);
      if (val.isstring)
        std::cout << val.s;
      else
        std::cout << val.d;
    } else {
      if (variables.count(token)) {
        VarInfo &v = variables[token];
        if (v.VT == VT_STRING)
          std::cout << v.s;
        else
          std::cout << v.d;
      } else {
        std::cout << "[undef:" << token << "]";
      }
    }
  }
  std::cout << std::endl;
}
else {
  size_t paren = token.find('(');
  if (paren != std::string::npos && token.back() == ')') {
    std::string name = token.substr(0, paren);
    std::string index_str = token.substr(paren + 1, token.size() - paren - 2);
    std::stringstream idxs(index_str);
    std::string n;
    std::vector<int> indices;
    while (std::getline(idxs, n, ','))
      indices.push_back(std::stoi(n));
    if (arrays.count(name)) {
      ArrayInfo &arr = arrays[name];
      if (indices.size() != arr.shape.size()) {
        std::cerr << "[?]";
      } else if (!arr.data.empty()) {
        int flat = 0, stride = 1;
        for (int i = indices.size() - 1; i >= 0; --i) {
          flat += indices[i] * stride;
          stride *= arr.shape[i];
        }
        std::cout << arr.data[flat];
      } else {
        std::cout << arr.sparse[indices];
      }
    } else {
      std::cerr << "[ERR]";
    }
  } else {
    try {
      return evaluateExpression(token, currentline);
    } catch (...) {
      std::cerr << "[ERR]";
    }
  }
}
first = false;
}
}
return std::endl;
}
