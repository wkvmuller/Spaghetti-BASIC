/ ========================= Statement Handlers =========================


void executeINPUT(const std::string &line) {
    // Parse optional prompt and variable list
    std::string rest = line.substr(5);
    std::string prompt;
    size_t semi = rest.find(';');
    if (semi != std::string::npos) {
        prompt = rest.substr(0, semi);
        rest = rest.substr(semi + 1);
    }
    // Trim prompt quotes
    if (!prompt.empty() && prompt.front() == '\"' && prompt.back() == '\"') {
        prompt = prompt.substr(1, prompt.size() - 2);
    }
    // Show prompt
    if (!prompt.empty()) std::cout << prompt << " ";
    std::cout << "? ";
    // Read user input line
    std::string inputLine;
    std::getline(std::cin, inputLine);

    // Tokenize input values
    std::vector<std::string> inputs;
    std::stringstream ssin(inputLine);
    std::string tok;
    while (std::getline(ssin, tok, ',')) {
        // trim whitespace
        tok.erase(0, tok.find_first_not_of(" \t"));
        tok.erase(tok.find_last_not_of(" \t") + 1);
        inputs.push_back(tok);
    }

    // Tokenize target variables
    std::vector<std::string> vars;
    std::stringstream ssvar(rest);
    while (std::getline(ssvar, tok, ',')) {
        tok.erase(0, tok.find_first_not_of(" \t"));
        tok.erase(tok.find_last_not_of(" \t") + 1);
        if (!tok.empty()) vars.push_back(tok);
    }

    // Assign values
    for (size_t i = 0; i < vars.size(); ++i) {
        // If not enough inputs, stop
        if (i >= inputs.size()) {
            std::cerr << "ERROR: Not enough input values." << std::endl;
            break;
        }
        const std::string &target = vars[i];
        const std::string &valstr = inputs[i];

        // Determine if string variable
        bool isStringVar = (!target.empty() && target.back() == '$");

        // Prepare ArgsInfo
        ArgsInfo value;
        if (isStringVar) {
            // literal or expression producing string
            if (valstr.size() >= 2 && valstr.front() == '\"' && valstr.back() == '\"') {
                value.isstring = true;
                value.s = valstr.substr(1, valstr.size() - 2);
            } else {
                // evaluate string function if needed
                value = evaluateStringFunction(valstr, { makeArgsInfo(currentline,"",false,"",0.0) });
            }
        } else {
            // numeric
            value.isstring = false;
            try {
                value.d = std::stod(valstr);
            } catch (...) {
                value.d = evaluateExpression(valstr, currentline);
            }
        }

        // Check for array indexing
        std::string name;
        std::vector<int> indices;
        if (parseIndexedArray(target, name, indices)) {
            setSparseValue(name, indices, value);
        } else {
            // Scalar variable
            VarInfo info;
            if (value.isstring) {
                info.vT = VT_STRING;
                info.s  = value.s;
            } else {
                info.vT = VT_DOUBLE;
                info.d  = value.d;
            }
            variables[target] = info;
        }
    }
}
