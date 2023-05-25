# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.8)

project(Final_AISever C CXX)

include(cmake/common.cmake model/model.hpp)

list(APPEND CMAKE_PREFIX_PATH "/home/leon/.local/libtorch_cpu/libtorch")
find_package(Torch REQUIRED)

# Proto file
get_filename_component(rs_proto "rs_protos/rs.proto" ABSOLUTE)
get_filename_component(rs_proto_path "${rs_proto}" PATH)

# Generated sources
set(rs_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/rs.pb.cc")
set(rs_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/rs.pb.h")
set(rs_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/rs.grpc.pb.cc")
set(rs_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/rs.grpc.pb.h")
add_custom_command(
      OUTPUT "${rs_proto_srcs}" "${rs_proto_hdrs}" "${rs_grpc_srcs}" "${rs_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${rs_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${rs_proto}"
      DEPENDS "${rs_proto}")


# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Include model
#add_subdirectory("${PROJECT_SOURCE_DIR}/model")

# target_link_libraries(${PROJECT_NAME} PUBLIC model)

# rs_grpc_proto
add_library(rs_grpc_proto
  ${rs_grpc_srcs}
  ${rs_grpc_hdrs}
  ${rs_proto_srcs}
  ${rs_proto_hdrs})
target_link_libraries(rs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})



# Targets RSer_[async_](client|server)
#foreach(_target
#  rser_server)
#  add_executable(${_target} "${_target}.cc")
#  target_link_libraries(${_target}
#    rs_grpc_proto
#    absl::flags
#    absl::flags_parse
#    ${_REFLECTION}
#    ${_GRPC_GRPCPP}
#    ${_PROTOBUF_LIBPROTOBUF})
#endforeach()

add_executable(${PROJECT_NAME} rser_server.cc)
target_link_libraries(${PROJECT_NAME}
        rs_grpc_proto
        absl::flags
        absl::flags_parse
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        ${TORCH_LIBRARIES}
        )