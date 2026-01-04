cmake_minimum_required(VERSION 3.14)
project(bgbinding)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Build the python extension module
pybind11_add_module(bgbinding bindings.cpp cpp_api.cpp)

target_link_libraries(bgbinding PRIVATE nlohmann_json::nlohmann_json)
