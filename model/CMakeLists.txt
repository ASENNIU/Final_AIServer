cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(RS_Server_demo)

list(APPEND CMAKE_PREFIX_PATH "/home/leon/.local/libtorch_cpu/libtorch")
find_package(Torch REQUIRED)


#add_executable(model_demo model_demo.cc)
add_library(model model.hpp)
target_link_libraries(model "${TORCH_LIBRARIES}")
set_property(TARGET model PROPERTY CXX_STANDARD 14)