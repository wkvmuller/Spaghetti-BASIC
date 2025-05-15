//
//==================================================================================
//    MAT Support functions
//

void sparseTrim(ArrayInfo &matrix) {
  for (auto it = matrix.sparse.begin(); it != matrix.sparse.end();) {
    if (std::abs(it->second) < 1e-12)
      it = matrix.sparse.erase(it);
    else
      ++it;
  }
}

double sparseSum(const ArrayInfo &matrix) {
  double total = 0.0;
  for (const auto &[_, val] : matrix.sparse)
    total += val;
  return total;
}

void sparseMultiplyScalar(ArrayInfo &matrix, double scalar) {
  for (auto &[_, val] : matrix.sparse)
    val *= scalar;
}

ArrayInfo sparseMask(const ArrayInfo &source, const ArrayInfo &mask) {
  ArrayInfo result = source;
  result.sparse.clear();
  for (const auto &[key, val] : source.sparse) {
    if (mask.sparse.count(key) && std::abs(mask.sparse.at(key)) > 1e-12)
      result.sparse[key] = val;
  }
  return result;
}

// Helper to parse index string like "A(1,2)" into name and index vector
bool parseIndexedArray(const std::string &token, std::string &name,
                       std::vector<int> &indices) {
  size_t open = token.find('(');
  size_t close = token.find(')');
  if (open == std::string::npos || close == std::string::npos || close < open)
    return false;
  name = token.substr(0, open);
  std::string indexPart = token.substr(open + 1, close - open - 1);
  std::istringstream iss(indexPart);
  std::string val;
  while (std::getline(iss, val, ',')) {
    indices.push_back(std::stoi(val));
  }
  return true;
}

// Retrieve numeric or string from sparse matrix
ArgsInfo getSparseValue(const std::string &name, const std::vector<int> &idx) {
  ArgsInfo result;
  if (arrays.count(name)) {
    if (arrays[name].sparse.count(idx)) {
      result.d = arrays[name].sparse.at(idx);
      result.isstring = false;
    } else if (arrays[name].stringSparse.count(idx)) {
      result.s = arrays[name].stringSparse.at(idx);
      result.isstring = true;
    } else {
      result.d = 0.0;
      result.s = "";
      result.isstring = false;
    }
  }
  return result;
}

// Assign value to sparse matrix
void setSparseValue(const std::string &name, const std::vector<int> &idx,
                    const ArgsInfo &value) {
  if (!arrays.count(name)) {
    arrays[name].dimensions = idx.size();
  }
  if (value.isstring) {
    arrays[name].stringSparse[idx] = value.s;
  } else {
    arrays[name].sparse[idx] = value.d;
  }
}

bool invertMatrix(const std::vector<double> &input, std::vector<double> &output,
                  int size) {
  output = input;
  std::vector<double> identity(size * size, 0.0);
  for (int i = 0; i < size; ++i)
    identity[i * size + i] = 1.0;

  for (int col = 0; col < size; ++col) {
    double diag = output[col * size + col];
    if (std::abs(diag) < 1e-12)
      return false;

    for (int j = 0; j < size; ++j) {
      output[col * size + j] /= diag;
      identity[col * size + j] /= diag;
    }

    for (int row = 0; row < size; ++row) {
      if (row != col) {
        double factor = output[row * size + col];
        for (int j = 0; j < size; ++j) {
          output[row * size + j] -= factor * output[col * size + j];
          identity[row * size + j] -= factor * identity[col * size + j];
        }
      }
    }
  }

  output = identity;
  return true;
}

double determinant(const std::vector<double> &mat, int n) {
  if (n == 1)
    return mat[0];
  if (n == 2)
    return mat[0] * mat[3] - mat[1] * mat[2];

  double det = 0.0;
  std::vector<double> submat((n - 1) * (n - 1));
  for (int col = 0; col < n; ++col) {
    int subi = 0;
    for (int i = 1; i < n; ++i) {
      int subj = 0;
      for (int j = 0; j < n; ++j) {
        if (j == col)
          continue;
        submat[subi * (n - 1) + subj] = mat[i * n + j];
        subj++;
      }
      subi++;
    }
    double sign = (col % 2 == 0) ? 1.0 : -1.0;
    det += sign * mat[col] * determinant(submat, n - 1);
  }
  return det;
}

void evaluateMATExpression(const std::string &target,
                           const std::string &expression) {
  if (expr.find("DETERMINANT(") == 0) {
    size_t open = expr.find("(");
    size_t close = expr.find(")");
    std::string source = expr.substr(open + 1, close - open - 1);
    if (arrays.find(source) == arrays.end()) {
      std::cerr << "ERROR: Matrix not found: " << source << std::endl;
      return;
    }
    const ArrayInfo &mat = arrays[source];
    if (mat.dimensions != 2 || mat.shape[0] != mat.shape[1]) {
      std::cerr << "ERROR: DETERMINANT requires a square 2D matrix.";
      return;
    }
    double resultVal = determinant(mat.data, mat.shape[0]);
    ArrayInfo result;
    result.dimensions = 2;
    result.shape = {1, 1};
    result.data = {resultVal};
    arrays[target] = result;
    return;
  }

  std::string expr = expression;
  expr.erase(0, expr.find_first_not_of(" 	"));

  if (expr.find("INV(") == 0) {
    size_t open = expr.find("(");
    size_t close = expr.find(")");
    std::string source = expr.substr(open + 1, close - open - 1);
    if (arrays.find(source) == arrays.end()) {
      std::cerr << "ERROR: INV source matrix not found: " << source
                << std::endl;
      return;
    }

    const ArrayInfo &src = arrays[source];
    if (src.dimensions != 2 || src.shape.size() != 2 ||
        src.shape[0] != src.shape[1]) {
      std::cerr << "ERROR: INV requires a square 2D matrix.";
      return;
    }

    ArrayInfo result;
    result.dimensions = 2;
    result.shape = src.shape;

    if (!invertMatrix(src.data, result.data, src.shape[0])) {
      std::cerr << "ERROR: INV matrix is singular or not invertible.";
      return;
    }

    arrays[target] = result;
    return;
  }

  if (expr.find("TRANS(") == 0) {
    size_t open = expr.find("(");
    size_t close = expr.find(")");
    std::string source = expr.substr(open + 1, close - open - 1);
    if (arrays.find(source) == arrays.end()) {
      std::cerr << "ERROR: TRANS source matrix not found: " << source
                << std::endl;
      return;
    }
    const ArrayInfo &src = arrays[source];
    if (src.dimensions != 2 || src.shape.size() != 2) {
      std::cerr << "ERROR: TRANS requires a 2D matrix." << std::endl;
      return;
    }

    ArrayInfo result;
    result.dimensions = 2;
    result.shape = {src.shape[1], src.shape[0]};
    result.data.resize(src.data.size());

    for (size_t r = 0; r < src.shape[0]; ++r) {
      for (size_t c = 0; c < src.shape[1]; ++c) {
        result.data[c * src.shape[0] + r] = src.data[r * src.shape[1] + c];
      }
    }

    arrays[target] = result;
    return;
  }

  std::istringstream iss(expr);
  std::string token1, op, token2;

  std::istringstream iss_check(expr);
  std::string left, op, right;
  iss_check >> left >> op >> right;
  if (op == "*" && arrays.find(left) == arrays.end() &&
      arrays.find(right) != arrays.end()) {
    // SCALAR * MATRIX
    double scalar = std::stod(left);
    const ArrayInfo &mat = arrays[right];
    ArrayInfo result = mat;
    if (mat.dimensions >= 4) {
      for (auto &[key, val] : result.sparse) {
        val *= scalar;
      }
    } else {
      for (auto &val : result.data) {
        val *= scalar;
      }
    }
    arrays[target] = result;
    return;
  } else if (op == "*" && arrays.find(left) != arrays.end() &&
             arrays.find(right) == arrays.end()) {
    // MATRIX * SCALAR
    double scalar = std::stod(right);
    const ArrayInfo &mat = arrays[left];
    ArrayInfo result = mat;
    if (mat.dimensions >= 4) {
      for (auto &[key, val] : result.sparse) {
        val *= scalar;
      }
    } else {
      for (auto &val : result.data) {
        val *= scalar;
      }
    }
    arrays[target] = result;
    return;
  }

  iss >> token1;

  if (iss >> op >> token2) {
    if (arrays.find(token1) == arrays.end() ||
        arrays.find(token2) == arrays.end()) {
      std::cerr << "ERROR: One or both matrices not defined: " << token1 << ", "
                << token2 << std::endl;
      return;
    }

    const ArrayInfo &a = arrays[token1];
    const ArrayInfo &b = arrays[token2];

    if (a.dimensions != b.dimensions || a.shape != b.shape) {
      std::cerr << "ERROR: Dimension mismatch in MAT operation." << std::endl;
      return;
    }

    ArrayInfo result;
    result.dimensions = a.dimensions;
    result.shape = a.shape;

    if (a.dimensions >= 4) {
      for (const auto &entry : a.sparse) {
        if (b.sparse.find(entry.first) != b.sparse.end()) {
          if (op == "+") {
            result.sparse[entry.first] =
                entry.second + b.sparse.at(entry.first);
          } else if (op == "-") {
            result.sparse[entry.first] =
                entry.second - b.sparse.at(entry.first);
          } else if (op == "*") {
            result.sparse[entry.first] =
                entry.second * b.sparse.at(entry.first);
          } else {
            std::cerr << "ERROR: Unsupported operator " << op
                      << " in sparse matrix." << std::endl;
            return;
          }
        }
      }
    } else {
      result.data.resize(a.data.size());
      for (size_t i = 0; i < result.data.size(); ++i) {
        if (op == "+") {
          result.data[i] = a.data[i] + b.data[i];
        } else if (op == "-") {
          result.data[i] = a.data[i] - b.data[i];
        } else if (op == "*") {
          result.data[i] = a.data[i] * b.data[i];
        } else {
          std::cerr << "ERROR: Unsupported operator " << op
                    << " in dense matrix." << std::endl;
          return;
        }
      }
    }

    arrays[target] = result;
  } else {
    if (arrays.find(token1) == arrays.end()) {
      std::cerr << "ERROR: Matrix not defined: " << token1 << std::endl;
      return;
    }
    arrays[target] = arrays[token1];
  }
}
