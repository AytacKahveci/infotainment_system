cmake_minimum_required(VERSION 3.14)
project(media_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_BINARY_DIR}/conan_toolchain.cmake")

find_package(gRPC CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(GTest REQUIRED)

get_filename_component(PROTO_DIR ${CMAKE_SOURCE_DIR}/../proto ABSOLUTE)
set(PROTO_FILE "${PROTO_DIR}/media.proto")
set(PROTO_GENERATED_DIR "${CMAKE_BINARY_DIR}")

add_custom_command(
  OUTPUT "${PROTO_GENERATED_DIR}/media.pb.cc" "${PROTO_GENERATED_DIR}/media.pb.h"
  COMMAND "protoc"
          "--proto_path=${PROTO_DIR}"
          "--cpp_out=${PROTO_GENERATED_DIR}"
          "${PROTO_FILE}"
  DEPENDS "${PROTO_FILE}"
  COMMENT "Generating Protobuf C++ code for ${PROTO_FILE}"
)

add_custom_command(
  OUTPUT "${PROTO_GENERATED_DIR}/media.grpc.pb.cc" "${PROTO_GENERATED_DIR}/media.grpc.pb.h"
  COMMAND "protoc"
          "--proto_path=${PROTO_DIR}"
          "--grpc_out=${PROTO_GENERATED_DIR}"
          "--plugin=protoc-gen-grpc=/home/aytac/.conan2/p/b/grpc11cd1577f43ea/p/bin/grpc_cpp_plugin"
          "${PROTO_FILE}"
  DEPENDS "${PROTO_FILE}"
  COMMENT "Generating gRPC C++ code for ${PROTO_FILE}"
)

set(MEDIA_GENERATED_SRCS
  "${PROTO_GENERATED_DIR}/media.pb.cc"
  "${PROTO_GENERATED_DIR}/media.grpc.pb.cc"
)
set(MEDIA_GENERATED_HDRS
  "${PROTO_GENERATED_DIR}/media.pb.h"
  "${PROTO_GENERATED_DIR}/media.grpc.pb.h"
)

include_directories(${CMAKE_SOURCE_DIR}/..
  "${PROTO_GENERATED_DIR}"
  ${gRPC_INCLUDE_DIRS}
  ${protobuf_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/media_service.cpp
  ${MEDIA_GENERATED_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    grpc::grpc
    protobuf::protobuf
)

add_executable(media_service_it
  test/media_service_it.cpp
  src/media_service.cpp
  ${MEDIA_GENERATED_SRCS}
)

target_link_libraries(media_service_it
  PRIVATE
    grpc::grpc
    protobuf::protobuf
    ${GTest_LIBRARIES}
)

add_custom_target(generate_media_proto
  DEPENDS ${MEDIA_GENERATED_SRCS} ${MEDIA_GENERATED_HDRS}
)
add_dependencies(${PROJECT_NAME} generate_media_proto)
