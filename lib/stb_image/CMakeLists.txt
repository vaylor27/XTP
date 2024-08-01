cmake_minimum_required(VERSION 3.28)
project(stb_image)

set(CMAKE_CXX_STANDARD 17)

add_library(stb_image INTERFACE stb_image.h)
add_library(XTP::stb ALIAS stb_image)

target_include_directories(stb_image INTERFACE .)