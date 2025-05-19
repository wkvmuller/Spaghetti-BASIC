#include "interpreter.h"
#include "program_structure.h"
#include <vector>


extern PROGRAM_STRUCTURE program;

extern int currentLine;

//typedef std::vector<std::vector<double>> Matrix;
//
//--------------------------------------------------------------------------------
//            Global Variables, structs, etc & helper functions.
//

//
//--------------------------------------------------------------------------------
//             prototypes
//

extern double evalExpression(const std::string &expr);
void executeCLOSE(const std::string &line);
void executeFORMAT(const std::string &);
void executeINPUT(const std::string &line);
void executeINPUTFILE(const std::string &line);
void executeINPUTSTRING(const std::string &line);
void executeOPEN(const std::string &line);
void executePRINT(const std::string &line, std::ostream &out);
void executePRINTFILE(const std::string &line);
void executePRINTFILEUSING(const std::string &line);
extern double evalExpression(const std::string &expr);
extern std::string trim(const std::string &s);
void executeMATPRINT(const std::string &line, std::ostream &out);
void executePRINTUSING(const std::string &line, std::ostream &out);

// ========================= Expression Evaluator =========================
//
void printMatrix(const std::string &name, int rows, int cols,
                 const Matrix* dense, const SparseMatrix* sparse) {
    std::cout << name << " =\n";
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double value = 0.0;
            if (dense) {
                if (i < dense->size() && j < (*dense)[i].size())
                    value = (*dense)[i][j];
            } else if (sparse) {
                auto it = sparse->find({i, j});
                if (it != sparse->end()) value = it->second;
            }
            std::cout << value << " ";
        }
        std::cout << "\n";
    }
}

void printMatrix(const std::string& name, const MatrixValue& mat) {
    if (mat.dimensions.size() != 2)
        throw std::runtime_error("PRINT only supports 2D matrices");

    int rows = mat.dimensions[0];
    int cols = mat.dimensions[1];

    std::cout << name << " =\n";
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            VarInfo v = mat.get({i, j});
            std::cout << v.numericValue << " ";
        }
        std::cout << "\n";
    }
}

/**
 * File-based MAT PRINT:
 *   MAT PRINT #<chan>, <matrixList>
 *
 * Parses the channel number, looks up the file stream, then
 * delegates to executeMATPRINT for the actual output.
 */
void executeMATPRINTFILE(const std::string &line) {
  static const std::regex rgx(R"(^\s*MAT\s+PRINT\s*#\s*(\d+)\s*,\s*(.+)$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid MAT PRINT #… syntax: " +
                             line);
  }

  // 1) Extract channel and matrix list
  int chan = std::stoi(m[1].str());
  std::string matList = m[2].str();

  // 2) Lookup the file stream handle
  auto it = program.fileHandles.find(chan);
  if (it == program.fileHandles.end() || !it->second.stream ||
      !it->second.stream->is_open()) {
    throw std::runtime_error("RUNTIME ERROR: File channel " +
                             std::to_string(chan) + " not open");
  }
  std::ostream &fileOut = *it->second.stream;

  // 3) Reconstruct a "MAT PRINT <matrixList>" line for printing
  std::string dispatchLine = "MAT PRINT " + matList;

  // 4) Delegate to the generic MAT PRINT handler
  executeMATPRINT(dispatchLine, fileOut);
}

/**
 * MAT PRINT handler supporting up to 15 dimensions.
 *
 * Syntax: MAT PRINT <name1>,<name2>,...
 * Prints each matrix as nested braces, e.g. {{1,2},{3,4}} for 2×2.
 * Higher-D matrices nest accordingly.
 */
void executeMATPRINT(const std::string &line, std::ostream &out = std::cout) {
  static const std::regex rgx(R"(^\s*MAT\s+PRINT\s+(.+)$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid MAT PRINT: " + line);
  }

  // Split comma-separated list of matrix names
  std::stringstream ss(m[1].str());
  std::string name;
  bool firstMatrix = true;

  while (std::getline(ss, name, ',')) {
    name = trim(name);
    // Lookup numeric matrix
    auto itNum = program.numericMatrices.find(name);
    if (itNum != program.numericMatrices.end()) {
      const MatrixValue &mv = itNum->second;
      if (!firstMatrix)
        out << '\n';
      firstMatrix = false;

      int D = int(mv.dimensions.size());
      if (D == 0) {
        out << "{}";
        continue;
      }
      if (D > 15) {
        throw std::runtime_error("RUNTIME ERROR: Matrix dimension >15 for " +
                                 name);
      }

      std::vector<int> idx(D);
      // Recursive lambda to print nested braces
      std::function<void(int)> rec = [&](int d) {
        out << '{';
        int dimSize = mv.dimensions[d];
        for (int i = 0; i < dimSize; ++i) {
          idx[d] = i;
          if (d + 1 < D) {
            rec(d + 1);
          } else {
            // Compute linear index for row-major order
            size_t lin = idx[0];
            for (int k = 1; k < D; ++k) {
              lin = lin * mv.dimensions[k] + idx[k];
            }
            VarInfo v;
            if (!mv.isSparse) {
              v = mv.denseValues[lin];
            } else {
              MatrixIndex mi{idx};
              auto fit = mv.sparseValues.find(mi);
              v = (fit != mv.sparseValues.end() ? fit->second : VarInfo());
            }
            out << v.numericValue;
          }
          if (i + 1 < dimSize)
            out << ',';
        }
        out << '}';
      };

      rec(0);
      continue;
    }

    // Lookup string matrix
    auto itStr = program.stringMatrices.find(name);
    if (itStr != program.stringMatrices.end()) {
      const MatrixValue &mv = itStr->second;
      if (!firstMatrix)
        out << '\n';
      firstMatrix = false;

      int D = int(mv.dimensions.size());
      if (D == 0) {
        out << "{}";
        continue;
      }
      if (D > 15) {
        throw std::runtime_error("RUNTIME ERROR: Matrix dimension >15 for " +
                                 name);
      }

      std::vector<int> idx(D);
      std::function<void(int)> rec = [&](int d) {
        out << '{';
        int dimSize = mv.dimensions[d];
        for (int i = 0; i < dimSize; ++i) {
          idx[d] = i;
          if (d + 1 < D) {
            rec(d + 1);
          } else {
            size_t lin = idx[0];
            for (int k = 1; k < D; ++k) {
              lin = lin * mv.dimensions[k] + idx[k];
            }
            VarInfo v;
            if (!mv.isSparse) {
              v = mv.denseValues[lin];
            } else {
              MatrixIndex mi{idx};
              auto fit = mv.sparseValues.find(mi);
              v = (fit != mv.sparseValues.end() ? fit->second : VarInfo());
            }
            out << v.stringValue;
          }
          if (i + 1 < dimSize)
            out << ',';
        }
        out << '}';
      };

      rec(0);
      continue;
    }

    throw std::runtime_error("RUNTIME ERROR: Matrix not found: " + name);
  }

  out << std::endl;
}

// Simple file‐PRINT (no USING)
void executePRINTFILE(const std::string &line) {
  static const std::regex rgx(R"(^\s*PRINT\s*#\s*(\d+)\s*,\s*(.*)$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid PRINT #… syntax: " + line);
  }

  // Extract channel and the rest of the print list
  int chan = std::stoi(m[1].str());
  std::string rest = m[2].str();

  // Lookup the file stream
  auto it = program.fileHandles.find(chan);
  if (it == program.fileHandles.end() || !it->second.stream ||
      !it->second.stream->is_open()) {
    throw std::runtime_error("RUNTIME ERROR: File channel " +
                             std::to_string(chan) + " not open");
  }
  std::ostream &out = *it->second.stream;

  // Delegate to the generic PRINT routine
  // Rebuild a line that starts with "PRINT " so executePRINT’s regex matches
  std::string printLine = "PRINT " + rest;
  executePRINT(printLine, out);
}

void executePRINTFILEUSING(const std::string &line) {
  static const std::regex rgx(
      R"(^\s*PRINT\s*#\s*(\d+)\s*,\s*USING\s+(\d+)\s+(.*)$)",
      std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid PRINT #… USING syntax: " +
                             line);
  }

  // 1) Extract channel, format‐line, and argument list
  int chan = std::stoi(m[1].str());
  std::string fmtLine = m[2].str();
  std::string args = m[3].str();

  // 2) Lookup the file stream
  auto it = program.fileHandles.find(chan);
  if (it == program.fileHandles.end() || !it->second.stream ||
      !it->second.stream->is_open()) {
    throw std::runtime_error("RUNTIME ERROR: File channel " +
                             std::to_string(chan) + " not open");
  }
  std::ostream &out = *it->second.stream;

  // 3) Reconstruct a “PRINT USING” line and delegate
  //    so that executePRINTUSING’s regex will match it.
  std::string dispatchLine = "PRINT USING " + fmtLine + " " + args;
  executePRINTUSING(dispatchLine, out);
}

/**
 * PRINT USING handler.
 *
 * Supports both console and file-redirected forms:
 *   PRINT USING <fmtLine> <expr1>,<expr2$>,...
 *   PRINT #<chan>, USING <fmtLine> <expr1>,...
 *
 * Always delegates formatting to this single function via the given ostream.
 */
void executePRINTUSING(const std::string &line, std::ostream &out) {
  static const std::regex rgx(
      R"(^\s*PRINT(?:\s*#\s*\d+\s*,)?\s+USING\s+(\d+)\s+(.*)\s*$)",
      std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid PRINT USING: " + line);
  }

  // 1) lookup the format spec
  int fmtLine = std::stoi(m[1].str());
  auto itFmt = program.printUsingFormats.find(fmtLine);
  if (itFmt == program.printUsingFormats.end()) {
    throw std::runtime_error("RUNTIME ERROR: FORMAT " +
                             std::to_string(fmtLine) + " not defined");
  }
  const std::string spec = itFmt->second;

  // 2) split the argument expressions
  std::vector<std::string> args;
  {
    std::stringstream ss(m[2].str());
    std::string tok;
    while (std::getline(ss, tok, ',')) {
      args.push_back(trim(tok));
    }
  }
  if (args.empty()) {
    throw std::runtime_error("RUNTIME ERROR: No arguments for PRINT USING");
  }

  // 3) walk the format string, one run == one argument
  size_t argIdx = 0;
  for (size_t i = 0; i < spec.size();) {
    char ch = spec[i];
    // if run of same code (#, L, C, R)
    if (ch == '#' || ch == 'l' || ch == 'L' || ch == 'c' || ch == 'C' ||
        ch == 'r' || ch == 'R') {
      char type = std::toupper(ch);
      size_t start = i;
      while (i < spec.size() && std::toupper(spec[i]) == type)
        ++i;
      int width = int(i - start);

      if (argIdx >= args.size()) {
        throw std::runtime_error(
            "RUNTIME ERROR: Not enough args for PRINT USING");
      }
      const std::string &expr = args[argIdx++];
      if (type == '#') {
        // numeric field: detect decimal precision
        std::string field = spec.substr(start, width);
        int prec = 0;
        auto dp = field.find('.');
        if (dp != std::string::npos)
          prec = int(field.size() - dp - 1);

        double val = evalExpression(expr);
        std::ostringstream oss;
        if (prec > 0)
          oss << std::fixed << std::setprecision(prec);
        oss << val;
        std::string s = oss.str();
        // right-justify
        if ((int)s.size() < width)
          out << std::string(width - s.size(), ' ');
        out << s;
      } else {
        // string field
        std::string s = evalStringExpression(expr);
        if ((int)s.size() > width)
          s.resize(width);

        int pad = width - int(s.size());
        if (type == 'L') {
          out << s << std::string(pad, ' ');
        } else if (type == 'R') {
          out << std::string(pad, ' ') << s;
        } else if (type == 'c') { // CENTER
          int left = pad / 2, right = pad - left;
          out << std::string(left, ' ') << s << std::string(right, ' ');
        } else {
          std::string mivic{type};
          std::string vmco = "RUNTIME ERROR: Invalid format \"" + mivic +
                             "\" for PRINT USING:" + spec;
          throw std::runtime_error(vmco);
        }
      }
    } else {
      // literal character
      out << ch;
      ++i;
    }
  }
  out << std::endl;
}
//
//=========================================================================
//  Statments support.
//

// extern PROGRAM_STRUCTURE program;
// OPEN statement: OPEN "filename" FOR INPUT|OUTPUT|APPEND AS #<channel>
void executeOPEN(const std::string &line) {
  static const std::regex rgx("^\\s*OPEN\\s*\"([^\"]+)\"\\s+FOR\\s+(INPUT|"
                              "OUTPUT|APPEND)\\s+AS\\s*#\\s*(\\d+)\\s*$",
                              std::regex::icase);

  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid OPEN: " + line);

  std::string path = m[1].str();
  std::string mode = m[2].str();
  int chan = std::stoi(m[3].str());

  std::ios_base::openmode om;
  if (mode == "INPUT")
    om = std::ios::in;
  else if (mode == "OUTPUT")
    om = std::ios::out | std::ios::trunc;
  else
    om = std::ios::out | std::ios::app;

  // Create or overwrite the stream on that channel
  auto &fh = program.fileHandles[chan];
  fh.stream.reset(new std::fstream(path, om));
  if (!fh.stream->is_open())
    throw std::runtime_error("RUNTIME ERROR: Cannot open file: " + path);
}

// statement: CLOSE #<channel>
void executeCLOSE(const std::string &line) {
  static const std::regex rgx(R"(^\s*CLOSE\s*#\s*(\d+)\s*$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid CLOSE: " + line);

  int chan = std::stoi(m[1].str());
  auto it = program.fileHandles.find(chan);
  if (it == program.fileHandles.end() || !it->second.stream)
    throw std::runtime_error("RUNTIME ERROR: File not open on channel " +
                             std::to_string(chan));

  it->second.stream->close();
  program.fileHandles.erase(it);
}

// FORMAT statement handler:
// Syntax: <lineNumber> := "<format-spec>"
// Stores the format spec for later PRINT USING calls.
void executeFORMAT(const std::string &line) {
  static const std::regex rgx(R"(^\\s*(\\d+)\\s*:=\\s*\\"([^\\"]*)\\"\\s*$)",
                              std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx)) {
    throw std::runtime_error("SYNTAX ERROR: Invalid FORMAT statement: " + line);
  }
  int fmtLine = std::stoi(m[1].str());
  std::string spec = m[2].str();
  program.printUsingFormats[fmtLine] = spec;
}

// Helper to find a line in programSource or throw
static std::map<int, std::string>::const_iterator findLine(int ln) {
  auto it = program.programSource.find(ln);
  if (it == program.programSource.end())
    throw std::runtime_error("RUNTIME ERROR: Undefined line " +
                             std::to_string(ln));
  return it;
}

// Shared helper for INPUT: read variables from an istream
static void processInputList(const std::string &vars, std::istream &in) {
  std::stringstream ss(vars);
  std::string v;
  while (std::getline(ss, v, ',')) {
    v = trim(v);
    if (v.empty())
      continue;
    bool isString = (!v.empty() && v.back() == '$');
    std::string name = isString ? v.substr(0, v.size() - 1) : v;
    VarInfo info;
    info.isArray = false;
    if (isString) {
      info.isString = true;
      if (!std::getline(in, info.stringValue))
        throw std::runtime_error("RUNTIME ERROR: Failed to read string for " +
                                 name);
      program.stringVariables[name] = info;
    } else {
      double d;
      if (!(in >> d))
        throw std::runtime_error("RUNTIME ERROR: Failed to read number for " +
                                 name);
      info.numericValue = d;
      info.isString = false;
      program.numericVariables[name] = info;
    }
  }
}

// Dispatcher for INPUT statements:
// 1) INPUT "prompt", varlist
// 2) INPUT #chan, varlist
// 3) INPUT varlist
void executeINPUTops(const std::string &line) {
  static const std::regex promptRe(
      R"(^\\s*INPUT\\s+\"([^\"]*)\"\\s*,\\s*(.+)$)", std::regex::icase);
  static const std::regex fileReInput(
      R"(^\\s*INPUT\\s*#\\s*(\\d+)\\s*,\\s*(.+)$)", std::regex::icase);
  static const std::regex varRe(R"(^\\s*INPUT\\s+(.+)$)", std::regex::icase);
static const std::regex inputRe(R"(^INPUT\s+([A-Z][A-Z0-9_]*)\((\d+),\s*(\d+)\)$)", std::regex::icase);

  std::smatch m;
  if (std::regex_match(line, m, promptRe)) {
    std::cout << m[1].str();
    processInputList(m[2].str(), std::cin);
  } else if (std::regex_match(line, m, fileReInput)) {
    int chan = std::stoi(m[1].str());
    auto it = program.fileHandles.find(chan);
    if (it == program.fileHandles.end() || !it->second.stream ||
        !it->second.stream->is_open())
      throw std::runtime_error("RUNTIME ERROR: File channel " +
                               std::to_string(chan) + " not open");
    processInputList(m[2].str(), *it->second.stream);
  } else if (std::regex_match(line, m, varRe)) {
    processInputList(m[1].str(), std::cin);
  } else 
  if (std::regex_match(line, m, inputRe)) {
    std::string name = m[1];
    int i = std::stoi(m[2]);
    int j = std::stoi(m[3]);

    std::cout << "?" << name << "(" << i << "," << j << ") = ";
    std::string response;
    std::getline(std::cin, response);

    double val = std::stod(response);
    VarInfo v = { val, false };
    program.matrices[name].set({i, j}, v);
}else {
    throw std::runtime_error("SYNTAX ERROR: Invalid INPUT statement: " + line);
  }
}
/**
 * Plain PRINT:
 *   PRINT <item1>,<item2$>,...
 * Writes to the given output stream.
 */
void executePRINT(const std::string &line, std::ostream &out) {
  static const std::regex rgx(R"(^\s*PRINT\s+(.*)$)", std::regex::icase);
  std::smatch m;
  if (!std::regex_match(line, m, rgx))
    throw std::runtime_error("SYNTAX ERROR: Invalid PRINT: " + line);

  std::string list = m[1].str();
  std::stringstream ss(list);
  std::string item;
  bool first = true;

  while (std::getline(ss, item, ',')) {
    item = trim(item);
    if (!first)
      out << ' ';
    first = false;

    // String literal?
    if (item.size() >= 2 && item.front() == '"' && item.back() == '"') {
      out << item.substr(1, item.size() - 2);
    }
    // String expression?
    else if (
        !item.empty() &&
        (item.back() == '$' ||
         std::regex_search(
             item,
             std::regex(
                 R"(\b(LEFT\$|RIGHT\$|MID\$|LEN\$|CHR\$|STRING\$|TIME\$|DATE\$)\b)",
                 std::regex::icase)))) {
      out << evalStringExpression(item);
    }
    // Numeric expression
    else {
      out << evalExpression(item);
    }
  }
  out << std::endl;
}

// Dispatcher for PRINT statements:
// 1) PRINT items to console
// 2) PRINT USING format to console
// 3) PRINT #chan, items to file
void executePRINTexpr(const std::string &line) {
  static const std::regex fileRePrint(
      R"PRINTF(^\s*PRINT\s*#\s*(\d+)\s*,)PRINTF", std::regex::icase);
  static const std::regex usingRe(R"USING(^\s*PRINT\s+USING\b)USING",
                                  std::regex::icase);
  static const std::regex printRe(R"PRINT(^\s*PRINT\b)PRINT",
                                  std::regex::icase);
  std::smatch m;
  if (std::regex_search(line, m, fileRePrint)) {
    int chan = std::stoi(m[1].str());
    auto it = program.fileHandles.find(chan);
    if (it == program.fileHandles.end() || !it->second.stream ||
        !it->second.stream->is_open())
      throw std::runtime_error("RUNTIME ERROR: File channel " +
                               std::to_string(chan) + " not open");
    executePRINT(line, *it->second.stream);
  } else if (std::regex_search(line, usingRe)) {
    executePRINTUSING(line, std::cout);
  } else if (std::regex_search(line, printRe)) {
    executePRINT(line, std::cout);
  } else {
    throw std::runtime_error("SYNTAX ERROR: Invalid PRINT statement: " + line);
  }

  // if (args.empty())
  //    throw std::runtime_error("RUNTIME ERROR: No arguments for PRINT USING");
}
