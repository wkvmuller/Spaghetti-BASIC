cmake_minimum_required(VERSION 3.25)
project(BasicRuntimeEnv CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_executable(basic_runtime_env
    main.cpp
    src/handle_list.cpp
    src/get_line_number.cpp
    src/memory.cpp
    src/parser.cpp
    src/interpreter.cpp
)
