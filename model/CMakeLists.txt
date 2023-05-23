cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(RS_Server_demo)

find_package(Torch REQUIRED)

add_executable(model_demo model_demo.cpp)
target_link_libraries(model_demo "${TORCH_LIBRARIES}")
set_property(TARGET model_demo PROPERTY CXX_STANDARD 14)