

# BasicInterpreter

A Dartmouth BASIC interpreter implemented in C++11. This interpreter accepts a BASIC.BAS file on the command line and executes BASIC programs according to our extended grammar.

## Features

- Command-line driven execution.
- Supports key BASIC commands (assignment, PRINT, IF, GOTO, REM, STOP, BREAK, etc.).
- Modular design enabling future extensions (FOR/NEXT loops, DO/LOOP blocks with nesting up to 15 levels, DIM, MAT commands, etc.).
- Built with CMake for cross-platform compatibility.

## Requirements

- C++11 compiler (e.g., g++)
- CMake 3.25 or later

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make

Usage

./basic_interpreter BASIC.BAS

Code Generation Credit

This project code was generated with assistance from ChatGPT, leveraging advanced language models to create a robust and scalable BASIC interpreter.
License

This project is licensed under the GNU GPL v3. See the LICENSE file for details.


### Strategic Overview

- **Credit Where It's Due:**  
  The README now acknowledges the role of ChatGPT in generating the interpreter code, reinforcing transparency and collaboration.

Deploy this README along with the rest of your project files to ensure proper attribution and maintain a professional repository structure.

