# BASIC Runtime Environment

This project implements a runtime environment for an extended version of Dartmouth BASIC V6.0. It includes:

- ✅ Full grammar definition (BNF/EBNF)
- ✅ Syntax checker (`syntax.cpp`) that validates structure, functions, and referenced line numbers
- ✅ Interpreter (`interpreter.cpp`) with:
  - Expression evaluation
  - Math functions including `LOGX(X,Y)`
  - Statement dispatch via `switch` structure
  - Executable `LET` and stub handlers for all other statements

## Project Structure

- `basic_runtime_env.cpp` — Main command loop with LOAD, LIST, SAVE, RUN, SYNTAX, NEW, etc.
- `syntax.cpp / syntax.h` — Full syntax validator
- `interpreter.cpp` — Expression-aware interpreter
- `BNF_with_LOGX.bnf` — Grammar specification including extensions
- `basic_test.bas` — Example source code to test syntax and runtime features

## Features

- ✔️ BNF-based design with extensions for:
  - Math and string functions
  - Control structures (WHILE/WEND, REPEAT/UNTIL)
  - Matrix operations (MAT)
  - File I/O (`OPEN`, `PRINT#`, `INPUT#`, `CLOSE`)
  - `SEED`, `BEEP`, `PRINT USING`, `FORMAT` lines

- ✔️ Syntax checker verifies:
  - Line references (GOTO, THEN, GOSUB, PRINT USING)
  - Matching loop pairs and nesting limits
  - Valid math and string function usage
  - Array dimensionality limits

- ✔️ Interpreter includes:
  - Recursive descent expression evaluation
  - Function call evaluation (`SIN`, `POW`, `SQR`, `LOGX`, etc.)
  - Variable storage and stub routing for full execution

## Attribution

Most of the code in this project was generated with the assistance of **ChatGPT**, using structured prompts and iterative refinement to build a complete and testable BASIC interpreter.

---

> Built for educational use, retrocomputing interest, and showcasing parser/runtime design patterns.
