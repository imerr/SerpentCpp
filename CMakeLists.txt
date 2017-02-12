cmake_minimum_required(VERSION 3.5)
project(SerpentCpp)

set(CMAKE_CXX_STANDARD 11)
include_directories("include")
set(SOURCE_FILES src/Serpent.cpp)
add_library(SerpentCpp ${SOURCE_FILES})