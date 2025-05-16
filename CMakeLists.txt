cmake_minimum_required(VERSION 3.25)
project(SpaghettiBASIC LANGUAGES CXX)

# Use C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Source files
set(SOURCES
    src/basic_runtime_env.cpp
    src/fileio.cpp
    src/interpreter.cpp
    src/renumber.cpp
    src/syntax.cpp
    src/program_structure.cpp
    src/InputOutput.cpp
    src/inlinefunctsupport.cpp
)

# Define the executable
add_executable(spaghetti_basic ${SOURCES})

# If you have additional libraries, link them here:
# target_link_libraries(spaghetti_basic PRIVATE <libraries...>)
