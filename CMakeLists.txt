cmake_minimum_required(VERSION 3.14)
project(bgbinding_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

pybind11_add_module(bgbinding bindings.cpp)

