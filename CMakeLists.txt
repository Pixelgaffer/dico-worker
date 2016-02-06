cmake_minimum_required(VERSION 2.8)
project(dico-worker)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support")
endif()

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SOURCE PROTO_HEADERS
        proto/do-task.proto
	proto/handshake.proto
	proto/self-describing-message.proto
	proto/task-result.proto
	proto/task-status.proto)

aux_source_directory(src WORKER_SOURCE)

add_executable(dico_worker ${WORKER_SOURCE} ${PROTO_SOURCE} ${PROTO_HEADERS})
target_link_libraries(dico_worker ${PROTOBUF_LIBRARIES})
