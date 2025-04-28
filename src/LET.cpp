
// ========================= Statement Handlers =========================

void executeLET(const std::string& line) {
    std::istringstream iss(line);
    std::string keyword, target, eq;
    iss >> keyword >> target >> eq;
    std::string expr;
    std::getline(iss, expr);
    expr.erase(0, expr.find_first_not_of(" \t"));

    // Determine if this is a string assignment
    bool isStringVar = (!target.empty() && target.back() == '$');

    // Evaluate the value
    ArgsInfo value;
    if (isStringVar && expr.size() >= 2 && expr.front() == '"' && expr.back() == '"') {
        // Literal string
        value.isstring = true;
        value.s = expr.substr(1, expr.size() - 2);
    } else {
        // Numeric or string-from-function
        if (isStringVar) {
            // Use string evaluator
            value = evaluateStringFunction(expr, { makeArgsInfo(currentline, "", false, expr, 0.0) });
        } else {
            // Numeric
            value.isstring = false;
            value.d = evaluateExpression(expr, currentline);
        }
    }

    // Check for array indexing
    std::string name;
    std::vector<int> indices;
    if (parseIndexedArray(target, name, indices)) {
        // Assign into array (dense vs sparse handled by ArrayInfo contents)
        setSparseValue(name, indices, value);
    } else {
        // Scalar variable
        VarInfo info;
        if (value.isstring) {
            info.vT = VT_STRING;
            info.s = value.s;
        } else {
            info.vT = VT_DOUBLE;
            info.d = value.d;
        }
        variables[target] = info;
    }

    // Echo assignment
    if (value.isstring)
        std::cout << target << " = \"" << value.s << "\"" << std::endl;
    else
        std::cout << target << " = " << value.d << std::endl;
}
