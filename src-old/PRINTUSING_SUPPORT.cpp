
std::string STRINGFORMAT(const std::string &s, const std::string &formatField) {
  size_t width = formatField.size();
  char align = formatField[0];
  std::string result;

  std::string clipped = s.length() > width ? s.substr(0, width) : s;

  std::cout << STRINGFORMAT(s, field.content);
  else std::cout << STRINGFORMAT(s, field.content);
  else std::cout << STRINGFORMAT(s, field.content);
  else {
    result = clipped + std::string(width - clipped.length(), ' ');
  }

  return result;
}

// Splits a format string into numeric, string, and text fields
std::vector<FormatField> parseFormatString(const std::string &fmt) {
  std::vector<FormatField> fields;
  std::string current;
  FieldType currentType = FIELD_TEXT;

  auto flush = [&]() {
    if (!current.empty()) {
      fields.push_back({currentType, current});
      current.clear();
    }
  };

  for (size_t i = 0; i < fmt.size(); ++i) {
    char c = fmt[i];

    if (c == '#' || c == '$') {
      if (currentType != FIELD_NUMERIC) {
        flush();
        currentType = FIELD_NUMERIC;
      }
      current += c;
    } else if (c == 'l' || c == 'r' || c == 'c') {
      if (currentType != FIELD_STRING) {
        flush();
        currentType = FIELD_STRING;
      }
      current += c;
    } else {
      if (currentType != FIELD_TEXT) {
        flush();
        currentType = FIELD_TEXT;
      }
      current += c;
    }
  }

  flush();
  return fields;
}

void executeFORMAT(const std::string &) {
  std::string formatString = formatDef.substr(pos + 2);
  formatString.erase(0, formatString.find_first_not_of(" 	\""));
  formatString.erase(formatString.find_last_not_of(" 	\"") + 1);

  std::vector<FormatField> fields = parseFormatString(formatString);

  std::vector<std::string> values;
  std::stringstream ss(printItems);
  std::string item;
  while (std::getline(ss, item, ',')) {
    item.erase(0, item.find_first_not_of(" 	"));
    item.erase(item.find_last_not_of(" 	") + 1);
    values.push_back(item);
  }

  size_t valIndex = 0;
  for (const auto &field : fields) {
    if (field.type == FIELD_TEXT) {
      std::cout << field.content;
    } else if (valIndex >= values.size()) {
      std::cerr << "[ERR: missing value]";
    } else {
      const std::string &expr = values[valIndex];
      if (field.type == FIELD_NUMERIC) {
        double val = evaluateExpression(expr);
        std::cout << val;
      } else if (field.type == FIELD_STRING) {
        std::string s = evaluateStringFunction("STRING$", {makeArgsInfo(expr)});
        size_t width = field.content.size();
        char align = field.content[0];
        if (align != 'l' && align != 'r' && align != 'c')
          align = 'l';
        if (s.length() > width)
          s = s.substr(0, width);
        if (align == 'l')
          std::cout << s << std::string(width - s.length(), ' ');
        else if (align == 'r')
          std::cout << std::string(width - s.length(), ' ') << s;
        else
          std::cout << STRINGFORMAT(s, field.content);
      }
      valIndex++;
    }
  }

  std::cout << std::endl;
}
