cmake_minimum_required(VERSION 3.25)
project(BasicRuntimeEnv CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS "-w")

add_executable(basic_runtime_env ${SOURCES})
