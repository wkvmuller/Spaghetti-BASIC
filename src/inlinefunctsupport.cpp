#include "program_structure.h"

//=======================================================================================
//   inline functsupport
//

double evaluateFunction(const std::string &name,
                        const std::vector<ArgsInfo> &args) {

  if (name == "ASCII")
    if (!args[0].isstring || args[0].s.empty()) {
      std::stringstream ss;

      ss << "Bad string passed to ASCII(" << args[0].s
         << ")  line:" << args[0].linenumber << std::endl;

      std::string token = ss.str();

      throw std::runtime_error(token);
    } else {
      return static_cast<double>(static_cast<unsigned char>(args[0].s[0]));
    }

  if (name == "LEN$")
    if (!args[0].isstring) {
      std::stringstream ss;
      ss << "bad non string passed to LEN$(" << args[0].d
         << ") on line: " << args[0].linenumber << std::endl;

      std::string token = ss.str();

      throw std::runtime_error(token);
    } else {
      return static_cast<double>(args[0].s.length());
    }

  if (name == "STRING$")
    if (!args[0].isstring)
      return static_cast<double>(std::stoi(args[0].s));

  if (name == "LOGX") {
    return std::log(args[1].d) / std::log(args[0].d);
  }
  if (name == "SIN") {
    return std::sin(args[0].d);
  }
  if (name == "COS") {
    return std::cos(args[0].d);
  }
  if (name == "TAN") {
    return std::tan(args[0].d);
  }
  if (name == "SQR") {
    return std::sqrt(args[0].d);
  }
  if (name == "LOG") {
    return std::log(args[0].d);
  }
  if (name == "LOG10" || name == "CLOG") {
    return std::log10(args[0].d);
  }
  if (name == "EXP") {
    return std::exp(args[0].d);
  }
  if (name == "INT") {
    return std::floor(args[0].d);
  }
  if (name == "ROUND") {
    return std::round(args[0].d);
  }
  if (name == "FLOOR") {
    return std::floor(args[0].d);
  }
  if (name == "CEIL") {
    return std::ceil(args[0].d);
  }
  if (name == "POW") {
    return std::pow(args[0].d, args[1].d);
  }
  if (name == "RND") {
    return static_cast<double>(rand()) / RAND_MAX;
  }
  if (name == "ASIN") {
    return std::asin(args[0].d);
  }
  if (name == "ACOS") {
    return std::acos(args[0].d);
  }
  if (name == "ATAN") {
    return std::atan(args[0].d);
  }
  if (name == "COT") {
    return 1.0 / std::tan(args[0].d);
  }
  if (name == "SEC") {
    return 1.0 / std::cos(args[0].d);
  }
  if (name == "CSC") {
    return 1.0 / std::sin(args[0].d);
  }
  if (name == "DEG2RAD") {
    return args[0].d * PI / 180.0;
  }
  if (name == "RAD2DEG") {
    return args[0].d * 180.0 / PI;
  }
  if (name == "DET") {
    std::stringstream ss;
    ss << "DET() not implemented - placeholder only." << std::endl;

    std::string token = ss.str();
    throw std::runtime_error(token);
  }
  std::stringstream ss;

  ss << "Unknown function: " << name << std::endl;

  std::string token = ss.str();
  throw std::runtime_error(token);
}

std::string evaluateStringFunction(const std::string &name,
                                   const std::vector<ArgsInfo> &args) {

  if (name == "TIME$") {
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", localtime(&now));
    return buffer;
  }

  if (name == "DATE$") {
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&now));
    return buffer;
  }

  if (name == "CHR$") {
    int c = static_cast<int>(args[0].d);
    if (c < 0 || c > 255) {
      return "";
    }
    return std::string(1, static_cast<char>(c));
  }

  if (name == "LEFT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0)
      n = 0;
    if (n > static_cast<int>(args[0].s.length()))
      n = args[0].s.length();
    return args[0].s.substr(0, n);
  }

  if (name == "RIGHT$") {
    int n = static_cast<int>(args[1].d);
    if (n < 0)
      n = 0;
    if (n > static_cast<int>(args[0].s.length()))
      n = args[0].s.length();
    return args[0].s.substr(args[0].s.length() - n);
  }

  if (name == "MID$") {
    int start = static_cast<int>(args[1].d);
    int len = static_cast<int>(args[2].d);
    if (start < 1)
      start = 1;
    if (len < 0)
      len = 0;
    if (start > static_cast<int>(args[0].s.length()))
      start = args[0].s.length();
    if (start - 1 + len > static_cast<int>(args[0].s.length()))
      len = args[0].s.length() - (start - 1);
    return args[0].s.substr(start - 1, len);
  }
  std::stringstream ss;

  ss << "Unknown function: " << name << std::endl;

  std::string token = ss.str();
  throw std::runtime_error(token);
}




// BEEP statement — emit a bell character
void executeBEEP(const std::string & /*line*/) {
  std::cout << '\a' << std::flush;
}

// DEF FN<name>(<param>) = <expression>
void executeDEF(const std::string &line) {
  static const std::regex rgx(
      R"(^\s*DEF\s+FN([A-Z][A-Z0-9_]{0,31})\s*\(\s*([A-Z][A-Z0-9_]{0,31})\s*\)\s*=\s*(.+)$)",
      std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid DEF: " + line);
  }

  std::string name = m[1].str();  // function name
  std::string param = m[2].str(); // single parameter
  std::string expr = m[3].str();  // body expression

  // Store or overwrite
  program.userFunctions[name] = UserFunction{param, expr};
}

void executeEND(const std::string &line) {
  throw std::runtime_error("RUNTIME ERROR: END of program");
}

// Assumes you have a helper to eval an arithmetic expression to an int:
