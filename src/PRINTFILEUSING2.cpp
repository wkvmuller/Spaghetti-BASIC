
void executePRINTFILEUSING(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, hashToken, usingToken;
    int filenum = -1, formatLine = -1;

    iss >> cmd >> hashToken >> filenum >> usingToken >> formatLine;

    std::string printItems;
    std::getline(iss, printItems);
    printItems.erase(0, printItems.find_first_not_of(" \t,"));

    if (!fileHandles.count(filenum) || !fileHandles[filenum].isFileOpen) {
        std::cerr << "ERROR: File #" << filenum << " is not open." << std::endl;
        return;
    }

    if (!programSource.count(formatLine)) {
        std::cerr << "ERROR: Format line " << formatLine << " not found." << std::endl;
        return;
    }

    std::string formatDef = programSource[formatLine];
    size_t pos = formatDef.find(":=");
    if (pos == std::string::npos) {
        std::cerr << "ERROR: Format line " << formatLine << " missing :=." << std::endl;
        return;
    }

    std::string formatString = formatDef.substr(pos + 2);
    formatString.erase(0, formatString.find_first_not_of(" \t\""));
    formatString.erase(formatString.find_last_not_of(" \t\"") + 1);

    std::vector<FormatField> fields = parseFormatString(formatString);

    std::vector<std::string> values;
    std::stringstream ss(printItems);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        values.push_back(item);
    }

    std::ostream& out = *(fileHandles[filenum].stream);
    size_t valIndex = 0;
    for (const auto& field : fields) {
        if (field.type == FIELD_TEXT) {
            out << field.content;
        } else if (valIndex >= values.size()) {
            out << "[ERR: missing value]";
        } else {
            const std::string& expr = values[valIndex];
            if (field.type == FIELD_NUMERIC) {
                double val = evaluateExpression(expr);
                out << val;
            } else if (field.type == FIELD_STRING) {
                ArgsInfo sval = evaluateStringFunction("STRING$", {makeArgsInfo(expr)});
                out << STRINGFORMAT(sval.s, field.content);
            }
            valIndex++;
        }
    }

    out << std::endl;
}
