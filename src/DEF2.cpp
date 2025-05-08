// Pseudocode for executeDEF
void executeDEF(const std::string &line) {
    // line looks like: "DEF FN X(A) = expression"
    std::istringstream iss(line);
    std::string tmp, fnKeyword, fname, params, eq, expr;
    iss >> tmp >> fnKeyword >> fname;           // tmp="DEF", fnKeyword="FN", fname="X(A)"
    size_t open = fname.find('('), close = fname.find(')');
    std::string name = fname.substr(0, open);   // "X"
    std::string arg  = fname.substr(open+1, close-open-1); // "A"
    iss >> eq;                                  // "="
    std::getline(iss, expr);                    // " expression"
    expr.erase(0, expr.find_first_not_of(" \t"));
    // Store in a map: function name → (arg name, expression text)
    userFns[name] = { arg, expr };
}
