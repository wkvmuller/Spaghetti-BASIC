cmake_minimum_required(VERSION 3.5)
project(SpaghettiBASIC LANGUAGES CXX)

# Enforce C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Source files
set(SOURCES
    src/program_structure.cpp
    src/basic_runtime_env.cpp
    src/fileio.cpp
    src/interpreter.cpp
    src/renumber.cpp
    src/syntax.cpp

    src/evalExpression.cpp
    src/evalStringExpression.cpp
)

# Define the executable
add_executable(spaghetti_basic ${SOURCES})
