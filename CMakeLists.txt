cmake_minimum_required(VERSION 3.25)
project(BasicRuntimeEnv CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(basic_runtime_env
    basic_runtime_env.cpp
    interpreter.cpp
    renumber.cpp
    syntax.cpp
)
