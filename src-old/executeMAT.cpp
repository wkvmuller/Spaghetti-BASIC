
void executeMAT(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, target, equals;
    iss >> cmd >> target >> equals;
    std::string expression;
    std::getline(iss, expression);
    expression.erase(0, expression.find_first_not_of(" \t"));
    evaluateMATExpression(target, expression);
}

// ─────────────────────────────────────────────────────────────────────────────
// Reads DATA statements into a previously DIM’d matrix (numeric only).
// Usage in BASIC:  MAT READ A
// It will collect all DATA values in program order and load them into A.

void executeMATREAD(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, name;
    iss >> cmd >> name;  // cmd == "MAT", next token is "READ", then name?
    // Some implementations write "MAT READ A". If yours uses that form:
    if (name == "READ") {
        iss >> name;
    }

    auto it = arrays.find(name);
    if (it == arrays.end()) {
        std::cerr << "ERROR: MAT READ on undefined matrix: " << name << std::endl;
        return;
    }
    ArrayInfo &mat = it->second;

    // Compute total elements = product of dimensions
    size_t total = 1;
    for (int d : mat.shape) {
        total *= d;
    }

    // Gather all DATA values
    std::vector<double> values;
    for (const auto &p : programSource) {
        std::istringstream ds(p.second);
        std::string kw;
        ds >> kw;
        if (kw == "DATA") {
            std::string rest;
            std::getline(ds, rest);
            std::stringstream vs(rest);
            std::string token;
            while (std::getline(vs, token, ',')) {
                // trim
                token.erase(0, token.find_first_not_of(" \t"));
                token.erase(token.find_last_not_of(" \t") + 1);
                try {
                    values.push_back(std::stod(token));
                } catch (...) {
                    values.push_back(0.0);
                }
            }
        }
    }

    if (values.size() < total) {
        std::cerr << "ERROR: MAT READ, not enough DATA values ("
                  << values.size() << " available for "
                  << total << " elements)" << std::endl;
        return;
    }

    // Load into dense storage (row-major)
    mat.data.clear();
    mat.data.reserve(total);
    for (size_t i = 0; i < total; ++i) {
        mat.data.push_back(values[i]);
    }
}

void executeMATPRINT(const std::string& line) {
    // Expect line of form “MAT PRINT A, B$, C”
    std::istringstream iss(line);
    std::string matKw, printKw;
    iss >> matKw >> printKw; // “MAT” and “PRINT”

    // Collect the matrix names
    std::vector<std::string> arraysToPrint;
    std::string name;
    while (std::getline(iss, name, ',')) {
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        if (!name.empty()) arraysToPrint.push_back(name);
    }

    // For each matrix, print all elements row by row
    for (const auto& arrName : arraysToPrint) {
        if (!arrays.count(arrName)) {
            std::cout << "[ERR: " << arrName << " undefined]\n";
            continue;
        }
        const ArrayInfo& mat = arrays.at(arrName);
        if (mat.shape.size() != 2) {
            std::cout << "[ERR: " << arrName << " not 2D]\n";
            continue;
        }
        int rows = mat.shape[0], cols = mat.shape[1];
        std::cout << "Matrix " << arrName << " (" << rows << "×" << cols << "):\n";
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                std::vector<int> idx = {r, c};
                // getSparseValue handles dense vs sparse, numeric vs string
                ArgsInfo v = getSparseValue(arrName, idx);
                if (v.isstring)        std::cout << v.s;
                else                   std::cout << v.d;
                if (c < cols - 1)      std::cout << "\t";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}
