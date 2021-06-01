cmake_minimum_required(VERSION 3.8)

project(TICHU_SERVER)

include(../../cmake/common.cmake)

get_filename_component(tichu_proto "../../proto/tichu_grpc_service.proto" ABSOLUTE)
get_filename_component(tichu_proto_path "${tichu_proto}" PATH)

set(tichu_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/tichu_grpc_service.pb.cc")
set(tichu_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/tichu_grpc_service.pb.h")
set(tichu_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/tichu_grpc_service.grpc.pb.cc")
set(tichu_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/tichu_grpc_service.grpc.pb.h")

add_custom_command(
        OUTPUT "${tichu_proto_srcs}" "${tichu_proto_hdrs}" "${tichu_grpc_srcs}" "${tichu_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${tichu_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${tichu_proto}"
        DEPENDS "${tichu_proto}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(tichu_grpc_proto
        ${tichu_grpc_srcs}
        ${tichu_grpc_hdrs}
        ${tichu_proto_srcs}
        ${tichu_proto_hdrs})
target_link_libraries(tichu_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})

add_executable(tichu-server tichu_server.cpp proto_converter.cpp)

target_link_libraries(tichu-server
        tichu_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})
