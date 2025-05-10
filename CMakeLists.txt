cmake_minimum_required(VERSION 3.25)
project(BASIC_Runtime_Env CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Source files
set(SOURCES
    basic_runtime_env_final_full_run.cpp
    interpreter_with_functions.cpp
    renumber_with_on.cpp
    syntax_with_on_final.cpp
    program_structure.cpp
)

# Create executable
add_executable(basic_runtime ${SOURCES})
