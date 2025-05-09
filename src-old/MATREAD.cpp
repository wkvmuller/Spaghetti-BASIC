void executeMATREAD(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, readWord, name;
    iss >> cmd >> readWord >> name;  // “MAT READ A”

    if (!arrays.count(name)) {
        std::cerr << "ERROR: MAT READ undefined matrix " << name << "\n";
        return;
    }
    ArrayInfo& mat = arrays[name];

    // Compute total elements
    size_t total = 1;
    for (int d : mat.shape) total *= d;

    // Temp index vector
    std::vector<int> idx(mat.shape.size());

    // Fill row-major order
    for (size_t n = 0; n < total; ++n) {
        // Convert flat n → multidimensional idx[]
        size_t rem = n;
        for (int dim = int(mat.shape.size()) - 1; dim >= 0; --dim) {
            idx[dim] = rem % mat.shape[dim];
            rem /= mat.shape[dim];
        }

        // Fetch next DATA item
        ArgsInfo v = getNextData();

        // Assign into dense or sparse, numeric or string
        if (!mat.data.empty()) {
            // Dense numeric
            mat.data[n] = v.isstring ? 0.0 : v.d;
        } else {
            // Sparse numeric
            if (!v.isstring)
                mat.sparse[idx] = v.d;
        }
        if (v.isstring) {
            // Store string regardless of dense/sparse
            mat.stringSparse[idx] = v.s;
        }
    }
}
